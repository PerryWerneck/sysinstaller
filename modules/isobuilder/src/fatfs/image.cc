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

// #ifdef HAVE_UNISTD_U
	#include <unistd.h>
// #endif // HAVE_UNISTD_U

 #ifdef HAVE_FATFS

 #include <fatfs.h>
 #include <fatfs/ff.h>
 #include <fatfs/diskio.h>

 using namespace Udjat;
 using namespace std;

 namespace FatFS {

	class Image::Disk : private Udjat::File::Temporary, public Reinstall::Abstract::Disk {
	private:
		FATFS fs;

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

			auto rc = f_mount(&fs, "0:", 1);
			if(rc != FR_OK) {
				throw runtime_error(Logger::Message{ _("Unexpected error '{}' on f_mount"), rc});
			}

		}

		virtual ~Disk() {
			auto rc = f_mount(NULL, "", 0);
			if(rc != FR_OK) {
				Logger::Message{ _("Unexpected error '{}' on f_mount"), rc}.error("fatfs");
			}
		}

	};

	Image::Image(const Udjat::Dialog &dialog, Reinstall::Builder &builder, const Settings &s)
		: Reinstall::Abstract::Image{dialog,builder}, settings{s}, disk{make_shared<Disk>(settings)} {
	}

	Image::~Image() {

	}

	void Image::append(std::shared_ptr<Reinstall::DataSource> source) {
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
	}

	void Image::post(Udjat::Abstract::Object &) {

	}

	void Image::write(Udjat::Dialog::Progress &progress) {

	}

 }
 #endif // HAVE_FATFS


