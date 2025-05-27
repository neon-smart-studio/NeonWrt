此專案已經不再維護轉而維護NeonOS
僅demo在Wrt系統上跑systemd

NeonWrt – 基於 Systemd 的 OpenWrt 發行版
-------------------------------------------------------------
* Systemd core + full boot flow support
* Targeted at Rockchip, Orange Pi, x86 SBCs, and IoT edge routers
* Full modular system with custom hostapd/dnsmasq/netifd support

如何使用
git clone https://github.com/neon-smart-studio/NeonWrt
cd NeonWrt

make menuconfig //選取你要的板子如樹霉派
![image](https://github.com/user-attachments/assets/89304293-4d9d-49c8-929f-0d4f7766e7b7)
![image](https://github.com/user-attachments/assets/7bc7e7d2-4bae-40c2-918c-f299508d5c68)
![image](https://github.com/user-attachments/assets/7003623c-718a-4ac9-b657-a6a7b490ce5c)

在advance devolpers options打勾
![image](https://github.com/user-attachments/assets/7b9ff821-e716-4080-ba1c-4233be046526)

進去advance devolpers options頁面勾選toolchain options
![image](https://github.com/user-attachments/assets/2760495c-b102-4edd-b583-ee6f692f38ca)

進去toolchain options頁面選取C Lib為glibc
![image](https://github.com/user-attachments/assets/f7294b1d-c584-42b1-9f7e-0242bd1fe1e2)

Exit後儲存

make -j28 V=99
等待大概半小時到一小時

在bin/targets/<你選的架構>/bcm2711-glibc
![image](https://github.com/user-attachments/assets/ea28eba8-662c-4255-b204-1ad5f111457c)

跑出來的結果(ssh)
![image](https://github.com/user-attachments/assets/79e5be19-599f-4753-b7e8-1ba7f041af0e)
