#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <ncurses.h>

// Pre-Req: 

// compile Command: gcc -Wall -c editor_2.c

// Build Command : gcc -Wall -o editor editor.c -pthread -lm -D_LARGE_THREADS -lncurses -lrt

#define ESC_CHARACTER '\033'
#define IS_KEY(A) ((A==KEY_UP||A==KEY_DOWN||A==KEY_RIGHT||A==KEY_LEFT)? 1:0)
#define WIDTH 100
#define HIGHT 1000
// Initialize cursor position
int x = 0, y = 0;
//int **length_of_file = NULL;
int *dirty_bit = NULL;
int terminate = 0;
int initial_val = 0;
int local_bit = 0;
void *file_attr= NULL;
pthread_mutex_t *mutex;
void **shared_memm=NULL;
void *saved_shm_ptr = NULL;
int saved_x,saved_y = 0;
int move_cursor = 0;
int c_count=0,t_width=0,t_height=0;
int row = 1;
void *shm_current = NULL;
void write_into_mem(char *ptr_shmem,char* input)
{
	*ptr_shmem = *input;
}

void* display_shared_mem(void *args)
{
	char *shared_mem = (char *)args;

	while (terminate ==0)
	{
	
		
		   if (*dirty_bit>0 || local_bit>0)
		   {
		   	        //clear();
			

				getyx(stdscr, y, x);
				move(0, 0);
				for (int i=0;i<*((int *)file_attr);i++) 
				{
					printw("%c",shared_mem[i]);



				}

				//move(y,0);	   
				move(y,c_count);
				refresh();

		
		   }
		   pthread_mutex_lock(mutex);
			if (*dirty_bit>0 && local_bit>0)
			   {
				local_bit--;
				//*dirty_bit=*dirty_bit-1;
			   }
			   else if  (local_bit<=0 && *dirty_bit>0)
			   {
			   	*dirty_bit=*dirty_bit-1;
			   }
			   else if (*dirty_bit<=0 && local_bit>0)
			   {
				local_bit--;
				//*dirty_bit=*dirty_bit-1;
			   }
		   
		   
		  pthread_mutex_unlock(mutex);

		sleep(0.03);
	}

return 0;
}

void *shared_mem_tail = NULL;
char key_operation(int ch,char **shm_current_ptr)
{
   getyx(stdscr, y, x);
	
	switch (ch) {
		
		case KEY_UP:
			if (y > 0) 
			{
				/*
				shm_current_ptr = (char **)(*shared_memm + (row-1)*WIDTH +c_count);
				row--;
				*/
				move(--y, c_count);
				
			}
			break;
		case KEY_DOWN:
			if (y < LINES - 1) {
				move(++y, c_count);
			}
			break;
		case KEY_LEFT:
		
			if (c_count > 0) {
			   *(char *)shm_current_ptr = *(char *)shm_current_ptr - sizeof(char);
			   c_count--;
			   
			   move(y, c_count);
			   refresh();

			   saved_x = x;
			   saved_y = y;
			   saved_shm_ptr = *shm_current_ptr;
			   move_cursor=1;
			}
			else if (c_count < 0)
			{
				if (y>0)
				{
				getmaxyx(stdscr, t_height, t_width); 
			        c_count = t_width;
				move(--y, c_count);
				}
			}
			break;
		case KEY_RIGHT:

				if (c_count < *(int *)file_attr) 
				{
				/*
				if (c_count>WIDTH)
				{
					y++;
					row++;
					c_count=0;
				}
				*/
				*(char *)shm_current_ptr = *(char *)shm_current_ptr + sizeof(char);
				saved_x = x;
			        saved_y = y;
			   	c_count++;
				
    				getmaxyx(stdscr, t_height, t_width); 
			        if (c_count>t_width)
			        {
			        	c_count=0;
			        	move(++y, c_count);
			  	}
			  	else
			  	{
			  	move(y, c_count);
			  	}
			        saved_shm_ptr = *shm_current_ptr;
			        move_cursor=1;
				}

			break;
		
		default:
			break;
	}
	refresh();
    return '0';
}


int main()
{
	int sh_mem_id=0;
	int mutex_id = 0;
	int file_attr_id=0;
	int dirty_bit_id=0;
	key_t shm_key = 1123;
	int shm_size = 1024;
	
	char *input = NULL;
	
	
	void *shared_mem = NULL;
		
	
	key_t file_attr_key = 1122;
	key_t mutex_key = 1125;
	key_t dirty_bit_key = 1126;
	pthread_mutexattr_t attr;
	input = (char *)malloc(sizeof(char));
	pthread_t thread_id;
	int local_len = 0;
	printf("Create New File ? Y/N \n");
	scanf("%c",input);
	printf("Selected %c \n\r",*input);
	if (*input == 'Y')
	{	
		//create shared memory segment
		sh_mem_id = shmget(shm_key,shm_size,0666|IPC_CREAT);
		
		file_attr_id = shmget(file_attr_key,4,0666|IPC_CREAT);
		
		mutex_id = shmget(mutex_key,sizeof(pthread_mutex_t),0666|IPC_CREAT);
		
		dirty_bit_id = shmget(dirty_bit_key,sizeof(int),0666|IPC_CREAT);

		printf("\rShared Memory Region Created with id %d \n\r",sh_mem_id);
	
	}
	else
	{
		free(input);
		input = (char *)malloc(50*sizeof(char));
		printf("Enter the file key to read/write \n\r");
		scanf("%s",input);

		
		// Covert this to integet
		sscanf(input,"%d",&shm_key);
		printf("File key is %d",shm_key);
		//open shared memory segment
		sh_mem_id = shmget(shm_key,shm_size,0666);	
		file_attr_id = shmget(file_attr_key,4,0666);
		mutex_id = shmget(mutex_key,sizeof(pthread_mutex_t),0666);
		dirty_bit_id = shmget(dirty_bit_key,sizeof(int),0666);
		printf("\rShared Memory Region Opened with %d \r\n",sh_mem_id);
	
	}
	if ((shared_mem = shmat(sh_mem_id, NULL, 0)) == (char *) -1) 
	{
        perror("Failed to attach");
        exit(1);
    }
    
	if ((file_attr = shmat(file_attr_id, NULL, 0)) == (char *) -1) 
	{
        perror("Failed to get file attributes");
        exit(1);
    }
	if ((dirty_bit = (int *)shmat(dirty_bit_id, NULL, 0)) == (int *) -1) 
	{
        perror("Failed to get file attributes");
        exit(1);
    }        
	if ((mutex = (pthread_mutex_t *)shmat(mutex_id, NULL, 0)) == (pthread_mutex_t *) -1) 
	{
        perror("Failed to get mutex object");
        exit(1);
    }
    	shared_memm = &shared_mem;
	//configure_terminal();
	shm_current = shared_mem;
	if (*input == 'Y')
	{
		*(int *)file_attr= 0;
		*dirty_bit=0;
				
				// initialize the mutex attributes
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

		// initialize the mutex in the shared memory segment
		pthread_mutex_init(mutex, &attr);
		for (int i=0;i<shm_size;i++)
		{
			*((char *)shared_mem+i)='\0';
		}
	}
	printw("File Length %d \n\r",*(int *)file_attr);
	
	    // Initialize ncurses
    initscr();
    // setting timeout
    timeout(500);
    // Enable arrow keys and keypad
    keypad(stdscr, TRUE);
    // Turn on cursor display
    curs_set(1);
    // Enable cursor coordinates tracking
    leaveok(stdscr, FALSE);

    refresh();
    noecho();  // don't print keys as they are typed
    cbreak();  

	
    pthread_create(&thread_id, NULL, display_shared_mem,shared_mem);

    	
	local_len = *(int *)file_attr;	 
	int c;
	//c = getch();
	c = ERR;
	//while (c==ERR)
	//{
	//	   c = getch();
	//}
	while (1)
	{
		
		while (c==ERR)
		{
		   c = getch();
		   //c='P';
		   sleep(0.01);
		   
		}
		if (c =='/')
		{
			break;
		}
		if (IS_KEY(c))
		{
			key_operation(c,(char **)&shm_current);
			
		}

		else
		{
			 if (c != '\n')
			 { 
				pthread_mutex_lock(mutex);
				//sleep(5);
				/*
				if (c_count>WIDTH)
				{
					y++;
					row++;
					c_count = 0;
					//char new_line = '\n';
					//write_into_mem((char *)shm_current,&new_line);
					//shm_current = shm_current + sizeof(char);
					move(y,c_count);
				}
				*/
				getmaxyx(stdscr, t_height, t_width); 
			        
				if (c_count>t_width)
			        {
			        	//printw("%d",c_count);
			        	//sleep(1);
			        	//c_count=0;
			        	//move(y,c_count);
				
			  	}
				if ((char *)shm_current >= ((char *)shared_mem+*(int *)file_attr))
				{
					*(int *)file_attr = *(int *)file_attr +1;
					local_len++;

				}				
	
				write_into_mem((char *)shm_current,(char*)&c);
				shm_current = shm_current + sizeof(char);
				
				c_count++;
				move(y,c_count);
				
						
				
				*dirty_bit+=1;
				local_bit+=1;
				initial_val=1;
				pthread_mutex_unlock(mutex);
				if ((*(int *)file_attr)==shm_size)
				{
				sleep(2);
				break;
				}
			}
		}
		c = ERR;
		//sleep(0.01);
/*
		c = ERR;
		while (c==ERR)
		{
		   c = getch();
		}
*/		
	}
	terminate =1;

	// Wait for the thread to complete before exiting the main thread
    pthread_join(thread_id, NULL);		  
    endwin();
    printf("%d",*(int *)file_attr);
    shmdt(shared_mem);
    shmdt(file_attr);
    shmdt(mutex);
	printf("\n\r Delete Shared Memory ? Y/N");
	free(input);
	input = (char *)malloc(sizeof(char));

	scanf("%s",input);
	printf("%s",input);
	if (*input == 'Y')
	{
		// Delete shared memory
		if (shmctl(sh_mem_id,IPC_RMID,NULL) == -1)
		{
			printf("Failed to delete shared memory region");
		}
		// Delete shared memory
		if (shmctl(file_attr_id,IPC_RMID,NULL) == -1)
		{
			printf("Failed to delete shared memory region");
		}
		// Delete shared memory
		if (shmctl(mutex_id,IPC_RMID,NULL) == -1)
		{
			printf("Failed to delete shared memory region");
		}
	}
}
