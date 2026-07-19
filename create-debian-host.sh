docker run -it \
	-p 8443:8443 \
	-p 6520:6520 \
	--privileged \
	--name sugarlemon \
	--device /dev/kvm \
	--device /dev/vhost-vsock \
	--device /dev/vsock \
	-v /dev/vhost-net:/dev/vhost-net \
	-v $(pwd)/cf_images:/images \
	debian:trixie bash