rm obj/arm/*
rm bin/arm/*
cp res/* bin/arm/res
arm-linux-gnueabi-g++ -mcpu=cortex-a8 `pkg-config --cflags --libs gtk+-2.0` -c -o obj/arm/explorer.o main.cpp
echo compiled
arm-linux-gnueabi-g++ -Xlinker obj/arm/explorer.o `pkg-config --cflags gtk+-2.0` -L/home/anakod/DEV/libs -lgtk-x11-2.0 -lgdk-x11-2.0 -lXrender -lXinerama -lXext -latk-1.0 -lcairo -lexpat -lffi -lfontconfig -lfreetype -lgdk_pixbuf-2.0 -lgio-2.0 -lglib-2.0 -lgmodule-2.0 -lgobject-2.0 -lgthread-2.0 -lpango-1.0 -lpangocairo-1.0 -lpangoft2-1.0 -lpixman-1 -lpng12 -lxcb-render -lxcb-shm -lXdamage -lXfixes -lz -lX11 -lxcb -lXau -lXdmcp -o bin/arm/explorer
echo builded
