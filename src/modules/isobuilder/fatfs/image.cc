/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 /**
  * @brief Implements fatfs image.
  */

 #include <config.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/tools/intl.h>
 #include <memory>
 #include <fcntl.h>
 #include <sys/stat.h>
 #include <reinstall/tools/datasource.h>
 #include <reinstall/disk/abstract.h>
 #include <reinstall/image.h>
 #include <reinstall/tools/writer.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #ifdef HAVE_FATFS

 #include <reinstall/modules/fatfs.h>
 #include <fatfs/ff.h>
 #include <fatfs/diskio.h>

 using namespace Udjat;
 using namespace std;

 namespace FatFS {

	class Image::Disk : private Udjat::File::Temporary, public Reinstall::Abstract::Disk {
	private:
		FATFS fs;
		bool mounted = false;

	public:
		Disk(const Settings &settings) : Reinstall::Abstract::Disk{Udjat::File::Handler::fd, settings.imglen} {

			if(disk_ioctl(0, CTRL_FORMAT, &fd) != RES_OK) {
				throw runtime_error(_("Cant bind fatfs to disk image"));
			}

			{

				// Format
				static const MKFS_PARM parm = {FM_FAT32, 0, 0, 0, 0};

				BYTE work[FF_MAX_SS];
				memset(work,0,sizeof(work));
				auto rc = f_mkfs("0:", &parm, work, sizeof work);

				if(rc != FR_OK) {
					throw runtime_error(Logger::Message{ _("Unexpected error '{}' on f_mkfs"), rc});
				}

			}

		}

		void mount() {
			auto rc = f_mount(&fs, "0:", 1);
			if(rc != FR_OK) {
				throw runtime_error(Logger::Message{ _("Unexpected error '{}' on f_mount"), rc});
			}
			mounted = true;
		}

		void unmount() {
			auto rc = f_unmount("0:");
			if(rc != FR_OK) {
				throw runtime_error(Logger::Message{ _("Unexpected error '{}' on f_umount"), rc});
			}
			mounted = false;
		}

		inline unsigned long long length() const {
			return File::Handler::length();
		}

		inline size_t block_size() const {
			return File::Handler::block_size();
		}

		inline size_t read(unsigned long long offset, void *contents, size_t length) {
			return File::Handler::read(offset,contents,length);
		}

		virtual ~Disk() {
			if(mounted) {
				Logger::Message{"Forcing unmount of FAT image"}.warning("fatfs");
				auto rc = f_unmount("0:");
				if(rc != FR_OK) {
					Logger::Message{ _("Unexpected error '{}' on f_umount"), rc}.error("fatfs");
				}
			}
		}

	};

	Image::Image(const Udjat::Dialog &dialog, Reinstall::Builder &builder, const Settings &s)
		: Reinstall::Abstract::Image{dialog,builder}, settings{s}, disk{make_shared<Disk>(settings)} {
	}

	Image::~Image() {

	}

	void Image::append(std::shared_ptr<Reinstall::DataSource> source) {

		const char *to = source->path();

		FIL fdst;
		{
			int rc;

			// Create directories
			{
				const char *last = strrchr(to,'/');
				const char *ptr = to;
				while(ptr < last) {
					const char *next = strchr(ptr+1,'/');
					string path{to,(size_t)(next-to)};

					rc = f_mkdir(path.c_str());
					if(rc != FR_OK && rc != FR_EXIST) {
						throw runtime_error(Logger::Message{_("Unable to create path fat://{} (rc={})"),path.c_str(),rc});
					}

					ptr = next;
				}
			}

			// Open file
			rc = f_open(&fdst, to, FA_WRITE | FA_CREATE_ALWAYS);
			if(rc != FR_OK) {
				throw runtime_error(Logger::Message{_("Unable to open fat://{} (rc={})"),to,rc});
			}

		}

		try {

			source->save(Udjat::Dialog::Progress::getInstance(),[&fdst,to](unsigned long long, unsigned long long, const void *buffer, size_t len){

				unsigned int wrote = 0;
				const BYTE *ptr = (const BYTE *) buffer;

				while(len > 0) {
					if(f_write(&fdst, ptr, (unsigned int) len, &wrote) != FR_OK) {
						throw runtime_error(Logger::Message{_("Unable to write fat://{}"),to});
					}
					len -= wrote;
					ptr += wrote;
				}

				return true;
			});

		} catch(...) {
			f_close(&fdst);
			throw;
		}

		f_close(&fdst);

	}

	void Image::append(const char *from, const char *to) {

		if(*to == '.') {
			to++;
		}

		FIL fdst;

		if(f_open(&fdst, to, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
			throw runtime_error(Logger::Message{_("Unable to open fat://{}"),to});
		}

		try {

			// http://elm-chan.org/fsw/ff/doc/open.html

			int fd = ::open(from,O_RDONLY);
			if(fd < 0) {
				throw system_error(errno, system_category(), from);
			}

			struct stat st;
			if(::fstat(fd,&st) != 0) {
				::close(fd);
				throw system_error(errno, system_category(), from);
			}

			if(st.st_blksize > 4096) {
				st.st_blksize = 4096;
			}

			BYTE buffer[st.st_blksize];

			unsigned int current = 0;
			while(current < st.st_size) {

				auto len = ::read(fd,buffer,st.st_blksize);
				if(len < 1) {
					::close(fd);
					throw system_error(errno, system_category(), from);
				}

				current += len;

				unsigned int wrote = 0;
				const BYTE *ptr = buffer;

				while(len > 0) {
					if(f_write(&fdst, ptr, (unsigned int) len, &wrote)  != FR_OK) {
						::close(fd);
						throw runtime_error(Logger::Message{_("Unable to write fat://{}"),to});
					}
					len -= wrote;
					ptr += wrote;
				}

			}

			::close(fd);

		} catch(...) {
			f_close(&fdst);
			throw;
		}

		f_close(&fdst);

	}

	void Image::pre(Udjat::Abstract::Object &) {
		Logger::String{"Opening disk image"}.info("fat");
		disk->mount();
	}

	void Image::post(Udjat::Abstract::Object &) {
		Logger::String{"Closing disk image"}.info("fat");
		disk->unmount();
	}

	void Image::write(Udjat::Dialog::Progress &progress) {

		Logger::String{"Preparing to write image"}.info("fat");
		progress = _( "Preparing to write" );

		unsigned long long total = disk->length();
		size_t buflen = disk->block_size();

		auto &writer = Reinstall::Writer::getInstance();
		writer.size(total);

		writer.open(progress,dialog);

		progress = _( "Writing image" );
		char buffer[buflen];

		unsigned long long current = 0LL;
		while(current < total) {

			progress.file_sizes(current,total);

			auto length = (total - current);
			if(length > buflen) {
				length = buflen;
			}

			auto bytes = disk->read(current,buffer,length);
			if(bytes == 0) {
				throw runtime_error("Unexpected EOF reading fat image");
			}

			writer.write(current, buffer, bytes);

			current += bytes;
		}

		progress.file_sizes(current,total);

		progress = _( "Finalizing" );
		writer.close();


	}

 }
 #endif // HAVE_FATFS


