
SYSTEMD_SYSTEM_TARGET_WANTS = \
	sockets.target.wants \
	runlevel1.target.wants \
	runlevel2.target.wants \
	runlevel3.target.wants \
	runlevel4.target.wants \
	runlevel5.target.wants \
	multi-user.target.wants \
	local-fs.target.wants \
	sysinit.target.wants

SYSTEMD_SYSTEM_TARGETS = \
	basic.target \
	bluetooth.target \
	ctrl-alt-del.target \
	default.target \
	emergency.target \
	final.target \
	getty.target \
	graphical.target \
	halt.target \
	initrd-fs.target \
	initrd-root-fs.target \
	initrd-switch-root.target \
	initrd.target \
	kexec.target \
	local-fs-pre.target \
	local-fs.target \
	machines.target \
	multi-user.target \
	network-online.target \
	nss-lookup.target \
	nss-user-lookup.target \
	paths.target \
	poweroff.target \
	printer.target \
	reboot.target \
	remote-fs-pre.target \
	remote-fs.target \
	rescue.target \
	rpcbind.target \
	runlevel0.target \
	runlevel1.target \
	runlevel2.target \
	runlevel3.target \
	runlevel4.target \
	runlevel5.target \
	runlevel6.target \
	shutdown.target \
	sigpwr.target \
	sleep.target \
	slices.target \
	smartcard.target \
	sockets.target \
	soft-reboot.target \
	sound.target \
	suspend.target \
	swap.target \
	sysinit.target \
	system-update.target \
	timers.target \
	time-sync.target \
	umount.target

SYSTEMD_SYSTEM_SLICES = 

SYSTEMD_SYSTEM_SOCKETS = \
	syslog.socket \
	systemd-initctl.socket \
	systemd-journald.socket \
	systemd-journald-audit.socket \
	systemd-journald-dev-log.socket

SYSTEMD_SYSTEM_PATHS = \
	systemd-ask-password-console.path \
	systemd-ask-password-wall.path

SYSTEMD_SYSTEM_SERVICES = \
	autovt@.service \
	console-getty.service \
	container-getty@.service \
	debug-shell.service \
	emergency.service \
	getty@.service \
	initrd-cleanup.service \
	initrd-parse-etc.service \
	initrd-switch-root.service \
	rc-local.service \
	rescue.service \
	serial-getty@.service \
	systemd-ask-password-console.service \
	systemd-ask-password-wall.service \
	systemd-fsck-root.service \
	systemd-fsck@.service \
	systemd-halt.service \
	systemd-initctl.service \
	systemd-journald.service \
	systemd-journal-catalog-update.service \
	systemd-journal-flush.service \
	systemd-kexec.service \
	systemd-poweroff.service \
	systemd-reboot.service \
	systemd-remount-fs.service \
	systemd-soft-reboot.service \
	systemd-suspend.service \
	systemd-sysctl.service \
	systemd-sysusers.service \
	systemd-tmpfiles-setup.service \
	systemd-tmpfiles-setup-dev.service \
	systemd-tmpfiles-setup-dev-early.service \
	systemd-udevd.service \
	systemd-udev-load-credentials.service \
	systemd-udev-trigger.service \
	systemd-update-done.service \
	systemd-update-utmp.service \
	user@.service

SYSTEMD_SYSTEM_MOUNTS = \
	dev-hugepages.mount \
	dev-mqueue.mount \
	sys-fs-fuse-connections.mount \
	sys-kernel-config.mount \
	sys-kernel-debug.mount \
	tmp.mount

SYSTEMD_UDEVD_SYSTEM_SERVICES = \
	initrd-udevadm-cleanup-db.service \
	systemd-hwdb-update.service \
	systemd-udev-settle.service \
	systemd-udev-trigger.service

SYSTEMD_UDEVD_SYSTEM_SOCKETS = \
	systemd-udevd-control.socket \
	systemd-udevd-kernel.socket

SYSTEMD_UDEVD_LIBS = \
	accelerometer \
	ata_id \
	cdrom_id \
	collect \
	mtd_probe \
	scsi_id \
	v4l_id