# Onix - 操作系统实现
- [项目地址](https://github.com/lunuj/Onix)
- [参考项目](https://github.com/StevenBaby/Onix)

## 主要功能
- 系统引导
  - [ ] bootloader

## Makefile说明
- make qemu-c
  - 编译烧写镜像并驱动gdb调试qemu
- make lib
  - 编译用户态程序使用的lib
- make utils [name]
  - 编译用户态工具程序，在src/builtin/utils目录下

## osh内置命令
- test
- logo
- pwd
- clear
- exit
- cd
- mkdir
- rmdir
- rm
- mount
- umount
- mkfs
- touc
- exec

## utils命令
- env
- ls
- cat