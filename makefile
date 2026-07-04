
all:
	@echo "\nDo nothing\n"

push_wiki:BUILD
	node doc.js
	cd wiki && git add . && git commit -m "wiki" && git push
	git add wiki && git commit -m "wiki" && git push

push:
	node doc.js
	cd wiki && git add . && git commit -m "wiki" || true && git push
	git add . && git commit -m "$(m)" && git push

merge:
	git checkout main
	git merge develop
	git push
	git checkout develop

buildSplash:
	$(CC) -g -fms-extensions -o build/arm64/splash splash.cpp -fpermissive -I.

splash:
	make buildSplash cc=arm64
	sshpass -p "$(PI_PASSWORD)" scp -v build/arm64/splash $(PI_TARGET):$(PI_REMOTE_DIR)/splash

stm32emu:
	cd mcu/stm32 && g++ emu.cpp -o emu -lsfml-graphics -lsfml-window -lsfml-system -lasound -lpthread -I../../. -I./Core/Inc/.
	cd mcu/stm32 && ./emu

stm32up:
	cd mcu/stm32 && make
	sudo dfu-util -a 0 -D mcu/stm32/build/rack.bin -s 0x08000000:leave
