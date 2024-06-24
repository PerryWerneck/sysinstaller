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
  * @brief Implement device writer.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <reinstall/tools/writer.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <stdexcept>

 #include <sys/ioctl.h>
 #include <sys/stat.h>
 #include <linux/fs.h>
 #include <sys/file.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <cstring>

 /*

	BLKGETSIZE
	Retrieve the size of the current device, expressed as the number of sectors. The value of arg passed by the system call is a pointer to a long value and should be used to copy the size to a user-space variable. This ioctl command is used, for instance, by mkfs to know the size of the filesystem being created.

	BLKFLSBUF
	Literally, ``flush buffers.'' The implementation of this command is the same for every device and is shown later with the sample code for the whole ioctl method.

	BLKRAGET
	Used to get the current read-ahead value for the device. The current value should be written to user space as a long item using the pointer passed to ioctl in arg.

	BLKRASET
	Set the read-ahead value. The user process passes the new value in arg.

	BLKRRPART
	Reread the partition table. This command is meaningful only for partitionable devices, introduced later in Section 12.7.

	BLKROSET , BLKROGET
	These commands are used to change and check the read-only flag for the device. They are implemented by the macro RO_IOCTLS(kdev_t dev, unsigned long where) because the code is device-independent. The macro is defined in blk.h.

 */


 using namespace Udjat;
 using namespace std;

 namespace Reinstall {

	int Writer::open(const char *device_name) {

		// Set device as R/W
		{
			int dfd = ::open(device_name,O_RDONLY);
			struct stat st;
			if(dfd > 0 && fstat(fd,&st) == 0 && (st.st_mode & S_IFMT) == S_IFBLK) {
				int state = 0;
				if(ioctl(dfd, BLKROSET, &state) == -1) {
					Logger::String{"Error '",strerror(errno),"' setting write permissions on ",device_name}.error("writer");
				} else {
					Logger::String{"Got write permission on ",device_name}.trace("writer");
				}
			}
			::close(dfd);
		}

		// Open device ...
		fd = ::open(device_name,O_RDWR);
		if(fd < 0) {
			return errno;
		}

		// ... and lock it.
		if(::flock(fd, LOCK_EX|LOCK_NB) < 0) {
			int err = errno;
			Logger::String{"Error '",strerror(errno)," getting lock on ",device_name}.error("writer");
			::close(fd);
			return err;
		}

		Logger::String{"Got lock on ",device_name}.trace("writer");

		return 0;
	}

	void Writer::close() {

		if(fd > 0) {
			::close(fd);
			fd = -1;
		}

	}

	bool Writer::allocate(unsigned long long length) {

		struct stat st;
		if(fd > 0 && fstat(fd,&st) == 0) {

			if((st.st_mode & S_IFMT) == S_IFBLK) {

				// Block device

				unsigned long long devlen = 0LL;
				if(ioctl(fd,BLKGETSIZE64,&devlen) < 0) {
					Logger::String{"Unable to get device length: ",strerror(errno)}.error("writer");
					return false;
				}

				if(devlen >= length) {
					return true;
				}

				throw runtime_error(_( "Not enough space on device") );

			} else if((st.st_mode & S_IFMT) == S_IFREG) {

				// Regular file
				if(fallocate(fd,0,0,length) == 0) {
					return true;
				}

				throw runtime_error(strerror(errno));

			}

		}

		return false;
	}

	unsigned long long Writer::size() const {

		unsigned long long devlen = 0LL;
		struct stat st;

		if(fd > 0 && fstat(fd,&st) == 0) {
			if((st.st_mode & S_IFMT) == S_IFBLK) {
				if(ioctl(fd,BLKGETSIZE64,&devlen) < 0) {
					Logger::String{"Unable to get device length: ",strerror(errno)}.error("writer");
				} else {
					devlen = 0LL;
				}
			}
		}

		return devlen;
	}

	void Writer::write(unsigned long long offset, const void *contents, unsigned long long length) {

		while(length > 0) {
			auto rc = pwrite(fd,contents,length,offset);
			if(rc < 0) {
				throw runtime_error(strerror(errno));
			} else if(rc > 0) {
				length -= rc;
				contents = (((uint8_t *) contents) + rc);
				offset += rc;
			}
		}

	}

 }
