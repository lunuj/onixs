#ifndef FS_H
#define FS_H

#include <onixs/types.h>
#include <onixs/list.h>

#define BLOCK_SIZE 1024 // 块大小
#define SECTOR_SIZE 512 // 扇区大小

#define MINIX1_MAGIC 0x137F // 文件系统魔数
#define NAME_LEN 14        // 文件名长度

#define BLOCK_BITS (BLOCK_SIZE * 8) // 块位图大小

#define BLOCK_INODES (BLOCK_SIZE / sizeof(inode_desc_t)) // 块 inode 数量
#define BLOCK_DENTRIES (BLOCK_SIZE / sizeof(dentry_t))   // 块 dentry 数量
#define BLOCK_INDEXES (BLOCK_SIZE / sizeof(uint16))         // 块索引数量

#define DIRECT_BLOCK (7)                                               // 直接块数量
#define INDIRECT1_BLOCK BLOCK_INDEXES                                  // 一级间接块数量
#define INDIRECT2_BLOCK (INDIRECT1_BLOCK * INDIRECT1_BLOCK)            // 二级间接块数量
#define TOTAL_BLOCK (DIRECT_BLOCK + INDIRECT1_BLOCK + INDIRECT2_BLOCK) // 全部块数量

#define SEPARATOR1 '/'
#define SEPARATOR2 '\\'
#define IS_SEPARATOR(c) (c == SEPARATOR1 || c == SEPARATOR2)

#define IMAP_NR 8
#define ZMAP_NR 8

enum file_flag
{
    O_RDONLY = 00,      // 只读方式
    O_WRONLY = 01,      // 只写方式
    O_RDWR = 02,        // 读写方式
    O_ACCMODE = 03,     // 文件访问模式屏蔽码
    O_CREAT = 00100,    // 如果文件不存在就创建
    O_EXCL = 00200,     // 独占使用文件标志
    O_NOCTTY = 00400,   // 不分配控制终端
    O_TRUNC = 01000,    // 若文件已存在且是写操作，则长度截为 0
    O_APPEND = 02000,   // 以添加方式打开，文件指针置为文件尾
    O_NONBLOCK = 04000, // 非阻塞方式打开和操作文件
};

typedef struct inode_desc_t
{
    uint16 mode;    // 文件类型和属性(rwx 位)
    uint16 uid;     // 用户id（文件拥有者标识符）
    uint32 size;    // 文件大小（字节数）
    uint32 mtime;   // 修改时间戳 这个时间戳应该用 UTC 时间，不然有瑕疵
    uint8 gid;      // 组id(文件拥有者所在的组)
    uint8 nlinks;   // 链接数（多少个文件目录项指向该i 节点）
    uint16 zone[9]; // 直接 (0-6)、间接(7)或双重间接 (8) 逻辑块号
} inode_desc_t;

typedef struct inode_t
{
    inode_desc_t *desc;   // inode 描述符
    struct buffer_t *buf; // inode 描述符对应 buffer
    dev_t dev;            // 设备号
    idx_t nr;             // i 节点号
    uint32 count;         // 引用计数
    time_t atime;         // 访问时间
    time_t ctime;         // 修改时间
    list_node_t node;     // 链表结点
    dev_t mount;          // 安装设备
} inode_t;

typedef struct super_desc_t
{
    uint16 inodes;        // 节点数
    uint16 zones;         // 逻辑块数
    uint16 imap_blocks;   // i 节点位图所占用的数据块数
    uint16 zmap_blocks;   // 逻辑块位图所占用的数据块数
    uint16 firstdatazone; // 第一个数据逻辑块号
    uint16 log_zone_size; // log2(每逻辑块数据块数)
    uint32 max_size;      // 文件最大长度
    uint16 magic;         // 文件系统魔数
} super_desc_t;

typedef struct super_block_t
{
    super_desc_t *desc;              // 超级块描述符
    struct buffer_t *buf;            // 超级快描述符 buffer
    struct buffer_t *imaps[IMAP_NR]; // inode 位图缓冲
    struct buffer_t *zmaps[ZMAP_NR]; // 块位图缓冲
    dev_t dev;                       // 设备号
    uint32 count;
    list_t inode_list;               // 使用中 inode 链表
    inode_t *iroot;                  // 根目录 inode
    inode_t *imount;                 // 安装到的 inode
} super_block_t;

// 文件目录项结构
typedef struct dentry_t
{
    uint16 nr;              // i 节点
    char name[NAME_LEN]; // 文件名
} dentry_t;

typedef dentry_t dirent_t;

typedef struct file_t
{
    inode_t *inode;
    uint32 count;
    off_t offset;
    int flags;
    int mode;
}file_t;

typedef enum whence_t
{
    SEEK_SET = 1,
    SEEK_CUR,
    SEEK_END
}whence_t;

super_block_t *get_super(dev_t dev);  // 获得 dev 对应的超级块
super_block_t *read_super(dev_t dev); // 读取 dev 对应的超级块

idx_t balloc(dev_t dev);          // 分配一个文件块
void bfree(dev_t dev, idx_t idx); // 释放一个文件块
idx_t ialloc(dev_t dev);          // 分配一个文件系统 inode
void ifree(dev_t dev, idx_t idx); // 释放一个文件系统 inode


idx_t bmap(inode_t *inode, idx_t block, bool create);

inode_t *get_root_inode();
inode_t *iget(dev_t dev, idx_t nr);
void iput(inode_t *inode);
inode_t *new_inode(dev_t dev, idx_t nr);

inode_t *named(char *pathname, char **next);
inode_t *namei(char *pathname);

// 从 inode 的 offset 处，读 len 个字节到 buf
int inode_read(inode_t *inode, char *buf, uint32 len, off_t offset);

// 从 inode 的 offset 处，将 buf 的 len 个字节写入磁盘
int inode_write(inode_t *inode, char *buf, uint32 len, off_t offset);

// 释放 inode 所有文件块
void inode_truncate(inode_t *inode);

// 打开文件，返回 inode
inode_t *inode_open(char *pathname, int flag, int mode);

void dev_init();
// 系统调用相关
// file.c
int sys_read(fd_t fd, char *buf, uint32 len);
int sys_write(fd_t fd, char *buf, uint32 len);

fd_t sys_open(char *filename, int flags, int mode);
void sys_close(fd_t fd);

fd_t sys_creat(char *filename, int mode);

int sys_lseek(fd_t fd, off_t offset, int whence);

int sys_readdir(fd_t fd, dirent_t *dir, uint32 count);

// namei.c
int sys_mknod(char *filename, int mode, int dev);
int sys_link(char *oldname, char *newname);
int sys_unlink(char *filename);
int sys_chdir(char *pathname);
int sys_mkdir(char *pathname, int mode);
int sys_rmdir(char *pathname);

int sys_chroot(char *pathname);

char *sys_getcwd(char *buf, size_t size);

// super.c
int sys_mount(char *devname, char *dirname, int flags);
int sys_umount(char *target);
int sys_mkfs(char *devname, int icount);
#endif // FS_H