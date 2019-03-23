/*     
struct stat {

        mode_t     st_mode;       //文件对应的模式，文件，目录等

        ino_t      st_ino;       //inode节点号

        dev_t      st_dev;        //设备号码

        dev_t      st_rdev;       //特殊设备号码

        nlink_t    st_nlink;      //文件的连接数

        uid_t      st_uid;        //文件所有者

        gid_t      st_gid;        //文件所有者对应的组

        off_t      st_size;       //普通文件，对应的文件字节数

        time_t     st_atime;      //文件最后被访问的时间

        time_t     st_mtime;      //文件内容最后被修改的时间

        time_t     st_ctime;      //文件状态改变时间

        blksize_t st_blksize;    //文件内容对应的块大小

        blkcnt_t   st_blocks;     //伟建内容对应的块数量

      };

stat结构体中的st_mode 则定义了下列数种情况：
    S_IFMT   0170000    文件类型的位遮罩
    S_IFSOCK 0140000    scoket
    S_IFLNK 0120000     符号连接
    S_IFREG 0100000     一般文件
    S_IFBLK 0060000     区块装置
    S_IFDIR 0040000     目录
    S_IFCHR 0020000     字符装置
    S_IFIFO 0010000     先进先出

    S_ISUID 04000     文件的(set user-id on execution)位
    S_ISGID 02000     文件的(set group-id on execution)位
    S_ISVTX 01000     文件的sticky位

    S_IRUSR(S_IREAD) 00400     文件所有者具可读取权限
    S_IWUSR(S_IWRITE)00200     文件所有者具可写入权限
    S_IXUSR(S_IEXEC) 00100     文件所有者具可执行权限

    S_IRGRP 00040             用户组具可读取权限
    S_IWGRP 00020             用户组具可写入权限
    S_IXGRP 00010             用户组具可执行权限

    S_IROTH 00004             其他用户具可读取权限
    S_IWOTH 00002             其他用户具可写入权限
    S_IXOTH 00001             其他用户具可执行权限

    上述的文件类型在POSIX中定义了检查这些类型的宏定义：
    S_ISLNK (st_mode)    判断是否为符号连接
    S_ISREG (st_mode)    是否为一般文件
    S_ISDIR (st_mode)    是否为目录
    S_ISCHR (st_mode)    是否为字符装置文件
    S_ISBLK (s3e)        是否为先进先出
    S_ISSOCK (st_mode)   是否为socket 
*/  

#include "myhead.h"


//保存文件个数
int file_num = 0;
//保存文件的大小
long int filesize = 0;
//保存已经拷贝了的文件的大小
long int copyfilesize = 0;

int GetFileSize( struct file *copy_file,thread_pool *pool)
{
	struct stat file_stat; 
	//获取文件的属性                 
	stat(copy_file->causefile,&file_stat); 

	//打开源目录  
	DIR *srcdir = opendir(copy_file->causefile); 
	struct dirent *dp;  

    //获取文件夹内文件的信息  
	while( (dp = readdir(srcdir))!=NULL ) 
	{  
		//如果文件为. 或者 .. 则跳过
		if(dp->d_name[0] == '.' )   
		{                          
			continue;  
		}  
  
		struct file *tmpfile = malloc(sizeof(struct file)); 
 		
 		//对内存清零 
		memset(tmpfile,0,sizeof(struct file));               
		//源文件路径
		sprintf(tmpfile->causefile,"%s/%s",copy_file->causefile,dp->d_name);  

		struct stat tmpstat;  
		stat(tmpfile->causefile,&tmpstat);     

		//如果为普通文件判断大小 
		if(S_ISREG(tmpstat.st_mode))                         
		{  			
			//计算单个文件的大小后相加
			long int newfilesize = tmpstat.st_size;
			//printf("文件 %s 的大小为 %ld \n",tmpfile->copyfile,newfilesize);
			filesize = filesize + newfilesize;

			//文件总个数+1
			file_num = file_num +1;
		}  

		//递归  
		else if(S_ISDIR(tmpstat.st_mode))
		{  
			GetFileSize(tmpfile,pool); 
		}  

	}  
	return 0;  
}

void *CopyFile(void *arg)
{
	//强转
	struct file *copy_file = (struct file *)arg;

	struct stat file_stat; 

	//通过文件名 获取文件的属性把他存放到结构体里面
	stat(copy_file->causefile, &file_stat);  
  
    //用只读的方式打开源文件                     
	int fd1 = open(copy_file->causefile,O_RDONLY);
	if(fd1 == -1 )  
	{  
       	printf("open file %s\n failed.\n",copy_file->causefile);  
       	return NULL;  
	}  
	 
	//以源文件的类型和权限创建文件      
	int fd2 = open(copy_file->copyfile,O_CREAT | O_TRUNC | O_RDWR,file_stat.st_mode); 
	                                                                            
	if( fd2 == -1)  
	{  
       	printf("open file %s failed.\n",copy_file->copyfile);  
       	return NULL;  
	}  
	  
	int n;  
	char buf[100];  

	//读取源文件的内容  拷贝

	while(1)  
	{  
		//把读到的全部写进目标文件
		n = read(fd1,buf,100);
		if(n == 0)
		{
			break;
		}
		
		write(fd2,buf,n);  
	}  
	  
	close(fd1);  
	close(fd2);  

	return NULL;  
}  
 
//拷贝目录,成功返回0.失败返回-1  
int copydir( struct file *copy_file,thread_pool *pool)  
{  
	struct stat file_stat; 
	//获取文件的属性                 
	stat(copy_file->causefile,&file_stat); 
	//以源目录的类型和目录来创建一个目录  
	mkdir(copy_file->copyfile,file_stat.st_mode);

	//打开源目录  
	DIR *srcdir = opendir(copy_file->causefile); 
	struct dirent *dp;  

 	//获取文件夹内文件的信息  
	while( (dp = readdir(srcdir))!=NULL )   
	{  
		//如果文件为. 或者 .. 则跳过
		if(strcmp(dp->d_name,".")==0 || strcmp(dp->d_name,"..")==0)   
		{                          
			continue;  
		}  

		struct file *tmpfile = malloc(sizeof(struct file)); 
 		
 		//对内存清零 
		memset(tmpfile,0,sizeof(struct file));               
		//源文件路径
		sprintf(tmpfile->causefile,"%s/%s",copy_file->causefile,dp->d_name);  
		//目标文件路径  
		sprintf(tmpfile->copyfile,"%s/%s",copy_file->copyfile,dp->d_name);

		struct stat tmpstat;  
		stat(tmpfile->causefile,&tmpstat);     

		//如果为普通文件,则拷贝 
		if(S_ISREG(tmpstat.st_mode))                         
		{  
			printf("拷贝文件 %s 成功! \n", tmpfile->copyfile); 
			
			//计算已经拷贝的文件相加后的大小
			long int newfilesize = tmpstat.st_size;
			copyfilesize = copyfilesize + newfilesize;

			//把复制的任务投入任务链表  
			add_task( pool, CopyFile, tmpfile);          
		}  

		//递归  
		else if(S_ISDIR(tmpstat.st_mode))
		{  
			copydir(tmpfile,pool);  
		}  

	}  

	return 0;  
}  

int main(int argc, char const *argv[])
{
	//1.确认参数总数是否为3,保存起始存储的时间
	if(argc != 3)
	{
		printf("copy error! plz input mycopy xxx xxx\n");
		exit(1);
	}
	int time1 = time(&copy_start_time);

	//2.初始化线程池 
	thread_pool *pool = malloc(sizeof(thread_pool));
	init_pool(pool,10); 	

	//3.把传入参数存入结构体
	struct file copy_file;
	strcpy(copy_file.causefile,argv[1]);
	strcpy(copy_file.copyfile,argv[2]);

	//4.判断需要拷贝的是文件还是目录
	struct stat info;
	stat(copy_file.causefile,&info);

	//获得目录的总大小与总个数
	GetFileSize(&copy_file,pool);

	//判断是否为文件拷贝
	if(S_ISREG(info.st_mode))
	{
		//创建线程
		printf("文件拷贝\n");
		pthread_t tid;
		pthread_create(&tid,NULL,CopyFile,&copy_file);
	} 
	//判断是否为目录拷贝
	else if(S_ISDIR(info.st_mode)) 
	{  
		printf("目录拷贝\n");
		copydir(&copy_file,pool); 
	}  
	else
	{
		printf("error!!!\n");
		exit(2);
	}

	//取消线程
	//remove_thread(pool, 10);

	//5.摧毁线程池
	destroy_pool(pool); 

	printf("copy success !!\n"); 
	int time2 = time(&copy_end_time);
	printf("文件个数为:%d\n",file_num);
	printf("拷贝总共用时:%d秒\n",time2 - time1);
	printf("文件的大小为:%ld字节\n",filesize);

	return 0;	
}
