#ifndef __MOUNT_H__
#define __MOUNT_H__

#define BLOCK_SIZE	1024
#define BLOCK_SIZE_BITS	10

/* These are the fs-independent mount-flags: up to 16 flags are
   supported  */
enum
{
  MS_RDONLY = 1,		/* Mount read-only.  */
#define MS_RDONLY	MS_RDONLY
  MS_NOSUID = 2,		/* Ignore suid and sgid bits.  */
#define MS_NOSUID	MS_NOSUID
  MS_NODEV = 4,			/* Disallow access to device special files.  */
#define MS_NODEV	MS_NODEV
  MS_NOEXEC = 8,		/* Disallow program execution.  */
#define MS_NOEXEC	MS_NOEXEC
  MS_SYNCHRONOUS = 16,		/* Writes are synced at once.  */
#define MS_SYNCHRONOUS	MS_SYNCHRONOUS
  MS_REMOUNT = 32,		/* Alter flags of a mounted FS.  */
#define MS_REMOUNT	MS_REMOUNT
  MS_MANDLOCK = 64,		/* Allow mandatory locks on an FS.  */
#define MS_MANDLOCK	MS_MANDLOCK
  MS_DIRSYNC = 128,		/* Directory modifications are synchronous.  */
#define MS_DIRSYNC	MS_DIRSYNC
  MS_NOSYMFOLLOW = 256,		/* Do not follow symlinks.  */
#define MS_NOSYMFOLLOW	MS_NOSYMFOLLOW
  MS_NOATIME = 1024,		/* Do not update access times.  */
#define MS_NOATIME	MS_NOATIME
  MS_NODIRATIME = 2048,		/* Do not update directory access times.  */
#define MS_NODIRATIME	MS_NODIRATIME
  MS_BIND = 4096,		/* Bind directory at different place.  */
#define MS_BIND		MS_BIND
  MS_MOVE = 8192,
#define MS_MOVE		MS_MOVE
  MS_REC = 16384,
#define MS_REC		MS_REC
  MS_SILENT = 32768,
#define MS_SILENT	MS_SILENT
  MS_POSIXACL = 1 << 16,	/* VFS does not apply the umask.  */
#define MS_POSIXACL	MS_POSIXACL
  MS_UNBINDABLE = 1 << 17,	/* Change to unbindable.  */
#define MS_UNBINDABLE	MS_UNBINDABLE
  MS_PRIVATE = 1 << 18,		/* Change to private.  */
#define MS_PRIVATE	MS_PRIVATE
  MS_SLAVE = 1 << 19,		/* Change to slave.  */
#define MS_SLAVE	MS_SLAVE
  MS_SHARED = 1 << 20,		/* Change to shared.  */
#define MS_SHARED	MS_SHARED
  MS_RELATIME = 1 << 21,	/* Update atime relative to mtime/ctime.  */
#define MS_RELATIME	MS_RELATIME
  MS_KERNMOUNT = 1 << 22,	/* This is a kern_mount call.  */
#define MS_KERNMOUNT	MS_KERNMOUNT
  MS_I_VERSION =  1 << 23,	/* Update inode I_version field.  */
#define MS_I_VERSION	MS_I_VERSION
  MS_STRICTATIME = 1 << 24,	/* Always perform atime updates.  */
#define MS_STRICTATIME	MS_STRICTATIME
  MS_LAZYTIME = 1 << 25,	/* Update the on-disk [acm]times lazily.  */
#define MS_LAZYTIME	MS_LAZYTIME
  MS_ACTIVE = 1 << 30,
#define MS_ACTIVE	MS_ACTIVE
  MS_NOUSER = 1 << 31
#define MS_NOUSER	MS_NOUSER
};

/* Flags that can be altered by MS_REMOUNT  */
#define MS_RMT_MASK (MS_RDONLY|MS_SYNCHRONOUS|MS_MANDLOCK|MS_I_VERSION \
		     |MS_LAZYTIME)


/* Magic mount flag number. Has to be or-ed to the flag values.  */

#define MS_MGC_VAL 0xc0ed0000	/* Magic flag number to indicate "new" flags */
#define MS_MGC_MSK 0xffff0000	/* Magic flag number mask */


/* Possible value for FLAGS parameter of `umount2'.  */
enum
{
  MNT_FORCE = 1,		/* Force unmounting.  */
#define MNT_FORCE MNT_FORCE
  MNT_DETACH = 2,		/* Just detach from the tree.  */
#define MNT_DETACH MNT_DETACH
  MNT_EXPIRE = 4,		/* Mark for expiry.  */
#define MNT_EXPIRE MNT_EXPIRE
  UMOUNT_NOFOLLOW = 8		/* Don't follow symlink on umount.  */
#define UMOUNT_NOFOLLOW UMOUNT_NOFOLLOW
};

// modeFlags and fsargs does not work as it is not implemented
int mount(const char* source, const char* target, const char* fstype, int modeFlags, char* fsargs);

#endif /*__MOUNT_H__*/