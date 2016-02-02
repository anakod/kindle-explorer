rm obj/arm/*
rm bin/arm/*
cp res/* bin/arm/res

#armv7l
arm-linux-gnueabi-g++ -mcpu=cortex-a8 `pkg-config --cflags --libs gtk+-2.0` -c -o obj/arm/explorer.o main.cpp
echo compiled
arm-linux-gnueabi-g++ -Xlinker obj/arm/explorer.o `pkg-config --cflags gtk+-2.0` -L/home/anakod/DEV/libs -lgtk-x11-2.0 -lgdk-x11-2.0 -lXrender -lXinerama -lXext -latk-1.0 -lcairo -lexpat -lffi -lfontconfig -lfreetype -lgdk_pixbuf-2.0 -lgio-2.0 -lglib-2.0 -lgmodule-2.0 -lgobject-2.0 -lgthread-2.0 -lpango-1.0 -lpangocairo-1.0 -lpangoft2-1.0 -lpixman-1 -lpng12 -lxcb-render -lxcb-shm -lXdamage -lXfixes -lz -lX11 -lxcb -lXau -lXdmcp -o bin/arm/explorer
echo builded

#armv6l
#edit USERNAME to match your username
#buildroot and libs from k3 required.
#CFLAGS="-U_FORTIFY_SOURCE -fno-stack-protector -O2 -fno-finite-math-only -ffast-math -march=armv6j -mtune=arm1136jf-s -mfpu=vfp -pipe -fomit-frame-pointer -fPIC" /opt/arm-2007q3/bin/arm-none-linux-gnueabi-g++ -W  main.cpp -o bin/arm/explorer `~/GIT/buildroot-2012.03/output/host/usr/bin/pkg-config gtk+-2.0 --cflags` -L/home/USERNAME/DEV/lib -lgtk-directfb-2.0 -lgdk-directfb-2.0 -lgdk_pixbuf-2.0 -lpangocairo-1.0 -latk-1.0 -lcairo -lpixman-1 -lpng12 -lgio-2.0 -lpangoft2-1.0 -lpango-1.0 -lfontconfig -lfreetype -lz -lexpat -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lglib-2.0 -ljpeg -ldirectfb-1.2 -lfusion -ldirect-1.2
