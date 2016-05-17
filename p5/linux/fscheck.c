#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/mman.h>

#define stat xv6_stat  // avoid clash with host struct stat
#define dirent xv6_dirent  // avoid clash with host struct stat
#include "types.h"
#include "fs.h"
#include "stat.h"
#undef stat
#undef dirent

#define BLOCK_SIZE (512)

int nblocks = 995;
int ninodes = 200;
int size = 1024;

int fsfd;
struct superblock sb;
char zeroes[512];
uint freeblock;
uint usedblocks;
uint bitblocks;
uint freeinode = 1;
uint root_inode;

void balloc(int);
void wsect(uint, void*);
void winode(uint, struct dinode*);
void rinode(uint inum, struct dinode *ip);
void rsect(uint sec, void *buf);
uint ialloc(ushort type);
void iappend(uint inum, void *p, int n);

int ar1[200], ar2[200], ar3[200], ar4[200], isDir[200];
int data_blk_beg, data_bitmap[1024];
void *img_ptr;
int root_dir_found = 0;
// convert to intel byte order
	ushort
xshort(ushort x)
{
	ushort y;
	uchar *a = (uchar*)&y;
	a[0] = x;
	a[1] = x >> 8;
	return y;
}

	uint
xint(uint x)
{
	uint y;
	uchar *a = (uchar*)&y;
	a[0] = x;
	a[1] = x >> 8;
	a[2] = x >> 16;
	a[3] = x >> 24;
	return y;
}


int 
mkfs(int nblocks, int ninodes, int size) {

	int i;
	char buf[BLOCK_SIZE];

	sb.size = xint(size);
	sb.nblocks = xint(nblocks); // so whole disk is size sectors
	sb.ninodes = xint(ninodes);

	bitblocks = size/(512*8) + 1;
	usedblocks = ninodes / IPB + 3 + bitblocks;
	freeblock = usedblocks;

	printf("used %d (bit %d ninode %zu) free %u total %d\n", usedblocks,
			bitblocks, ninodes/IPB + 1, freeblock, nblocks+usedblocks);

	assert(nblocks + usedblocks == size);

	for(i = 0; i < nblocks + usedblocks; i++)
		wsect(i, zeroes);

	memset(buf, 0, sizeof(buf));
	memmove(buf, &sb, sizeof(sb));
	wsect(1, buf);


	return 0;
}

int
add_dir(DIR *cur_dir, int cur_inode, int parent_inode) {
	int r;
	int child_inode;
	int cur_fd, child_fd;
	struct xv6_dirent de;
	struct dinode din;
	struct dirent dir_buf;
	struct dirent *entry;
	struct stat st;
	int bytes_read;
	char buf[BLOCK_SIZE];
	int off;

	bzero(&de, sizeof(de));
	de.inum = xshort(cur_inode);
	strcpy(de.name, ".");
	iappend(cur_inode, &de, sizeof(de));

	bzero(&de, sizeof(de));
	de.inum = xshort(parent_inode);
	strcpy(de.name, "..");
	iappend(cur_inode, &de, sizeof(de));

	if (cur_dir == NULL) {
		return 0;
	}

	cur_fd = dirfd(cur_dir);
	if (cur_fd == -1){
		perror("add_dir");
		exit(EXIT_FAILURE);
	}

	if (fchdir(cur_fd) != 0){
		perror("add_dir");
		return -1;
	}

	while (true) {
		r = readdir_r(cur_dir, &dir_buf, &entry);

		if (r != 0) {
			perror("add_dir");
			return -1;
		}

		if (entry == NULL)
			break;

		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		printf("%s\n", entry->d_name);

		child_fd = open(entry->d_name, O_RDONLY);
		if (child_fd == -1) {
			perror("open");
			return -1;
		}

		r = fstat(child_fd, &st);
		if (r != 0) {
			perror("stat");
			return -1;
		}

		if (S_ISDIR(st.st_mode)) {
			child_inode = ialloc(T_DIR);
			r = add_dir(fdopendir(child_fd), child_inode, cur_inode);
			if (r != 0) return r;
			if (fchdir(cur_fd) != 0) {
				perror("chdir");
				return -1;
			}
		} else {
			bytes_read = 0;
			child_inode = ialloc(T_FILE);
			bzero(&de, sizeof(de));
			while((bytes_read = read(child_fd, buf, sizeof(buf))) > 0) {
				iappend(child_inode, buf, bytes_read);
			}
		}
		close(child_fd);

		de.inum = xshort(child_inode);
		strncpy(de.name, entry->d_name, DIRSIZ);
		iappend(cur_inode, &de, sizeof(de));

	}

	// fix size of inode cur_dir
	rinode(cur_inode, &din);
	off = xint(din.size);
	off = ((off/BSIZE) + 1) * BSIZE;
	din.size = xint(off);
	winode(cur_inode, &din);
	return 0;
}




int isBadInode(struct dinode *iptr){
	if(iptr->type <0 || iptr->type > 3){
		return 1;
	}
	return 0;
}

int walkInodeAddr(struct dinode *iptr){
	//assuming valid inode type
	
	struct xv6_dirent *entry;
	int cur_inum;
	if(iptr->type == 0)
		return 1;
		
	int i;
	for(i=0;i<=NDIRECT;i++){
		if(iptr->addrs[i] != 0 && (iptr->addrs[i] < data_blk_beg || iptr->addrs[i] > 1023)){
			fprintf(stderr,"ERROR: bad address in inode.\n");
			return 0;
		}
		
		//check for data block bitmap in use for every iptr->addrs[i]
		int block_no = iptr->addrs[i];

		//int bit_val = bitmap_buf[block_no/8] & (0x1 << (block_no%8));
		//reset datablock bitmap
		//printf("block discovered %d\n", block_no);
		int bitmap_val = data_bitmap[block_no];

		if(block_no > 0){	
			//printf("unmarking %d\n",block_no);
			data_bitmap[block_no]++;
		}
		if(block_no > 0 && data_bitmap[block_no] > 2 ){
			fprintf(stderr,"ERROR: address used more than once.\n");
			return 0;
		}	

		if(iptr->addrs[i] > 0 && !bitmap_val){
        		fprintf(stderr,"ERROR: address used by inode but marked free in bitmap.\n");
        		return 0;
		}

		if(iptr->type == 1){  //DIR
			//read the 1st data block of the directory to check for . and ..
			int blk_num = iptr->addrs[i];
			entry = (struct xv6_dirent *)(img_ptr + (blk_num*BSIZE));
			int k = 0, num_entries = 512/sizeof(struct xv6_dirent);
			if(i == 0){
				if(strcmp(entry->name,".")!=0 || strcmp((entry+1)->name, "..") !=0 ){
					fprintf(stderr,"ERROR: directory not properly formatted.\n");
					return 0;		
				}	
				if(!root_dir_found && entry->inum == 1 && (entry+1)->inum == 1)
					root_dir_found = 1;
				int parent_inum = (entry+1)->inum;	
				cur_inum = entry->inum;
				isDir[cur_inum] = 1;
				//printf("cur dir=%d parent=%d\n",entry->inum, (entry+1)->inum);
				//store the parent of inode obtained from ..
				ar1[cur_inum] = parent_inum;		
				entry+=2; //bypass . and ..
				k+=2;
			}
			if(i<NDIRECT){ //excluding indirect block
				for(;k<num_entries;k++,entry++){
					if(entry->inum!=0){
						ar2[entry->inum] = cur_inum;
						ar4[entry->inum]++;
					}
				}	
			}
		}

	}
	//check blocks pointed by indirect block
	int num_indirect = 512/sizeof(uint);
	int blk_no = iptr->addrs[NDIRECT];
	uint *ptr = (uint *) (img_ptr + (blk_no*BSIZE));
	for(i=0;i<num_indirect;i++,ptr++){
		if(*ptr != 0 && (*ptr < data_blk_beg || *ptr > 1023)){
			fprintf(stderr,"ERROR: bad address in inode.\n");
			return 0;
		}
		
		if(*ptr > 0 && !data_bitmap[*ptr]){
        		fprintf(stderr,"ERROR: address used by inode but marked free in bitmap.\n");
        		return 0;
		}
		//reset datablock bitmap
		//printf("block discovered %d\n", *ptr);
		if(*ptr > 0){
			//printf("unmarking %d\n", *ptr);
			data_bitmap[*ptr]++;
		}
		if(data_bitmap[*ptr] > 2){
			fprintf(stderr,"ERROR: address used more than once.\n");
			return 0;
		}
				
		//read data block if directory
		if(iptr->type == 1){
			
			entry = (struct xv6_dirent *)(img_ptr + (*ptr * BSIZE));
			int k = 0, num_entries = 512/sizeof(struct xv6_dirent);
			for(;k<num_entries && entry->inum!=0;k++,entry++){
				ar2[entry->inum] = cur_inum;
				ar4[entry->inum]++;
			}
		}
	}
	
	return 1;
}


int walkInodeTable(){
	int inode_block_count = ninodes/IPB;
	int blk = 2;
	int i;
	for(i=0;i<ninodes;i++){
		ar1[i] = 0;
		ar2[i] = 0;

	}
	ar2[1] = 1; //root dir inode 
	ar4[1] = 1;
	//bitmap 
	int bit_map_blk = ninodes / IPB + 3;
	char* bitmap_buf = (char *)(img_ptr + (bit_map_blk*BSIZE));

	//walk thro bitmap block and mark all used ones
	int ctr;
	for(ctr = data_blk_beg; ctr < size; ctr++){
        	data_bitmap[ctr] = ((bitmap_buf[ctr/8] & (0x1 << (ctr%8))) > 0)?1:0; //assign the appropriate bit value for the block
		/*if(ctr == 500){
			printf("state of 500 =%d\n",data_bitmap[ctr]);
		}*/
	}
	int inode_indx = 0;
	for(; blk < 2+inode_block_count;blk++){
		//rsect(blk,buf);
		struct dinode *inptr = (struct dinode *)(img_ptr + (blk*BSIZE));
		//int i;
		for(i=0;i<IPB;i++,inptr++){
			//process inode
			if(isBadInode(inptr)){
				fprintf(stderr,"ERROR: bad inode.\n");
				return 0;
			}
			if(walkInodeAddr(inptr) == 0){
				return 0;
			}
			//printf("in use inode %d\n",inode_indx);
			if(inptr->type != 0){ //inode is used
				ar3[inode_indx] = (inptr->nlink > 1)?inptr->nlink:1;	
			}
			//traversed 1st inode and still didnt find root_dir inode
			if(inode_indx == 1 && !root_dir_found){
				fprintf(stderr,"ERROR: root directory does not exist.\n");
				return 0;
			}
			inode_indx++;
		}	
	} 
	//compare for parent-child dir relationship
	for(i=0;i<ninodes;i++){
		if(isDir[i] == 1 ){ //&& !(ar3[i] > 1 || ar4[i] > 1) && ar1[i]!=ar2[i]){ 
			if(isDir[i] == 1 && (ar3[i] > 1 || ar4[i] > 1)){
				fprintf(stderr,"ERROR: directory appears more than once in file system.\n");
				return 0;
			}
			if(ar1[i]!=ar2[i]){
			//more than 1 ref link to dir also causes parent dir mismatch
				fprintf(stderr,"ERROR: parent directory mismatch.\n");
				return 0;
			}
		}
		//check for inode usage discrepancy
		if(ar3[i] > 0 && ar4[i] == 0){
			fprintf(stderr,"ERROR: inode marked use but not found in a directory.\n");
			return 0;
		}
		else if(ar3[i] == 0 && ar4[i] > 0){
			fprintf(stderr,"ERROR: inode referred to in directory but marked free.\n");
			return 0;
		}
		if(isDir[i] == 0 && ar3[i] != ar4[i]){
			fprintf(stderr,"ERROR: bad reference count for file.\n");
			return 0;
		}/*
		else if(isDir[i] == 1 && (ar3[i] > 1 || ar4[i] > 1)){
			fprintf(stderr,"ERROR: directory appears more than once in file system.\n");
			return 0;
		}*/
	}

	//check in bitmap after wlaking thro inode table
	//if any of the datablocks in bitmap still 1 then throw error
	for(i=data_blk_beg;i<data_blk_beg+nblocks+1;i++){
        	if(data_bitmap[i] == 1){
                	fprintf(stderr,"ERROR: bitmap marks block in use but it is not in use.\n");
                	return 0;
        	}
	}


	
			
	return 1;
}

int
main(int argc, char *argv[])
{
	int rc;

	if(argc < 2){
		fprintf(stderr, "Usage: fscheck fs.img files...\n");
		exit(1);
	}
	fsfd = open(argv[1], O_RDONLY) ;
	if(fsfd < 0){
		fprintf(stderr,"image not found.\n");
        	exit(1);
	}

	struct stat sbuf;
	rc = fstat(fsfd, &sbuf);
	assert(rc == 0);
	img_ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fsfd, 0);
	
	//read superblock
	struct superblock *sblock;
	sblock = (struct superblock *)(img_ptr + BSIZE);
	/*printf("size=%d\n",sblock->size);  
	printf("nblocks=%d\n",sblock->nblocks);
	printf("ninodes=%d\n",sblock->ninodes);
	*/

	uint bitblcks = sblock->size/(512*8) + 1;
	data_blk_beg = ninodes / IPB + 3 + bitblcks;  
	//printf("data block beg=%d\n", data_blk_beg);
	int ret_val = walkInodeTable();
	if(ret_val == 0)
		exit(1);
	
	//printf("completed successfully\n");
	exit(0);
}

void
wsect(uint sec, void *buf)
{
	if(lseek(fsfd, sec * 512L, 0) != sec * 512L){
		perror("lseek");
		exit(1);
	}
	if(write(fsfd, buf, 512) != 512){
		perror("write");
		exit(1);
	}
}

	uint
i2b(uint inum)
{
	return (inum / IPB) + 2;
}

	void
winode(uint inum, struct dinode *ip)
{
	char buf[512];
	uint bn;
	struct dinode *dip;

	bn = i2b(inum);
	rsect(bn, buf);
	dip = ((struct dinode*)buf) + (inum % IPB);
	*dip = *ip;
	wsect(bn, buf);
}

	void
rinode(uint inum, struct dinode *ip)
{
	char buf[512];
	uint bn;
	struct dinode *dip;

	bn = i2b(inum);
	rsect(bn, buf);
	dip = ((struct dinode*)buf) + (inum % IPB);
	*ip = *dip;
}

	void
rsect(uint sec, void *buf)
{
	if(lseek(fsfd, sec * 512L, 0) != sec * 512L){
		perror("lseek");
		exit(1);
	}
	if(read(fsfd, buf, 512) != 512){
		perror("read");
		exit(1);
	}
}

	uint
ialloc(ushort type)
{
	uint inum = freeinode++;
	struct dinode din;

	bzero(&din, sizeof(din));
	din.type = xshort(type);
	din.nlink = xshort(1);
	din.size = xint(0);
	winode(inum, &din);
	return inum;
}

	void
balloc(int used)
{
	uchar buf[512];
	int i;

	printf("balloc: first %d blocks have been allocated\n", used);
	assert(used < 512*8);
	bzero(buf, 512);
	for(i = 0; i < used; i++){
		buf[i/8] = buf[i/8] | (0x1 << (i%8));
	}
	printf("balloc: write bitmap block at sector %zu\n", ninodes/IPB + 3);
	wsect(ninodes / IPB + 3, buf);
}

#define min(a, b) ((a) < (b) ? (a) : (b))

	void
iappend(uint inum, void *xp, int n)
{
	char *p = (char*)xp;
	uint fbn, off, n1;
	struct dinode din;
	char buf[512];
	uint indirect[NINDIRECT];
	uint x;

	rinode(inum, &din);

	off = xint(din.size);
	while(n > 0){
		fbn = off / 512;
		assert(fbn < MAXFILE);
		if(fbn < NDIRECT){
			if(xint(din.addrs[fbn]) == 0){
				din.addrs[fbn] = xint(freeblock++);
				usedblocks++;
			}
			x = xint(din.addrs[fbn]);
		} else {
			if(xint(din.addrs[NDIRECT]) == 0){
				// printf("allocate indirect block\n");
				din.addrs[NDIRECT] = xint(freeblock++);
				usedblocks++;
			}
			// printf("read indirect block\n");
			rsect(xint(din.addrs[NDIRECT]), (char*)indirect);
			if(indirect[fbn - NDIRECT] == 0){
				indirect[fbn - NDIRECT] = xint(freeblock++);
				usedblocks++;
				wsect(xint(din.addrs[NDIRECT]), (char*)indirect);
			}
			x = xint(indirect[fbn-NDIRECT]);
		}
		n1 = min(n, (fbn + 1) * 512 - off);
		rsect(x, buf);
		bcopy(p, buf + off - (fbn * 512), n1);
		wsect(x, buf);
		n -= n1;
		off += n1;
		p += n1;
	}
	din.size = xint(off);
	winode(inum, &din);
}
