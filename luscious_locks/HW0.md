#Chapter1

1. 
```c
#include <unistd.h>
int main() {
	write(1, "Hi! My name is Zhentian Zhang\n", 30);
	return 0;
}
```

2. 
```c
#include <unistd.h>
void write_triangle(int n);
int main() {
	write_triangle(45);
	return 0;
}
void write_triangle(int n) {
	int i;
	for (i = 1; i <= n; i++) {
		int j = i;
		while(j) {
			write(2, ".", 1);
			j--;
		}
		write(2, "\n", 1);
	}
}
```

3. 
```c
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
	mode_t mode = S_IRUSR | S_IWUSR;
	int fildes = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
	write(fildes, "Hi! My name is Zhentian Zhang\n", 30);
	close(fildes);
	return 0;
}
```

4. 
```c
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
	fclose(stdout);
	mode_t mode = S_IRUSR | S_IWUSR;
	int fildes = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
	printf("%s", "Hi! My name is Zhentian Zhang\n");
	close(fildes);
	return 0;
}
```

5. 
Write is a system call inside the kernel. It provides the very basic write funcionality, which only write a sequence of bytes. Printf allows writing data in different formats easily. It is a higher-level function. Printf outputs to stdout by default. 


#Chapter2

1. 
At least 8 bits. 

2. 
1

3. 
int: 4
double: 8
float: 4
long: 4
long long: 8

4. 
0x7fbd9d50

5. 
*(data+8)

6. 
The "hello" is read-only, and line2 is trying to modify it, which is not allowed. 

7. 
12

8. 
5

9. 
sizeof("aa") = 3

10. 
sizeof(int)


#Chapter3

1. 
argc and count it using a loop.

2. 
The program name. 

3. 
Somewhere else. They live above stack

4. 
sizeof(ptr) = 4
sizeof(array) = 6

5. 
stack

#Chapter4

1. 
We can use malloc to allocate memory, which will live on the heap. It has to be freed after done using it. 
Also, we can use static variable, which will last for the entire process. 

2. 
Stack memory will be allocated when a function is called, and freed after the function is finished. 
Heap memory is memory we can allocate manually, and it lives for the whole process. 

3. 
There are stack, heap, data, text. 

4. 
free

5. 
There is no enough memory to allocate. 

6. 
time() is the time since January 1, 1970.
ctime() will return a string representation of the time. 

7. 
You can't free the same ptr twice. 

8. 
you can't use the pointer after it has been freed.

9. 
Set the ptr to null.

10. 
```c
struct Person {
	char* name;
	int age;
	struct Person** friends;
};
typedef struct Person Person;
```

11. 
```c
#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

struct Person {
	char* name;
	int age;
	struct Person** friends;
};
typedef struct Person Person;

Person* create(char* name, int age) {
	Person* p = (Person*) malloc(sizeof(Person));
	p->friends = malloc(sizeof(Person*) * 10);
	p->name = NULL;
	int i;
	for (i = 0; i < 10; i++) {
		p->friends[i] = NULL;
	}
	p->name = strdup(name);
	p->age = age;
	return p;
}

void destroy(Person* p) {
	free(p->name);
	free(p->friends);
	memset(p, 0, sizeof(Person));
	free(p);
}

int main() {
	Person* Agent = create("Agent Smith", 128);
	Person* Sonny = create("Sonny Moore", 256);
	
	Person* A_friends[1];
	A_friends[0] = Sonny;
	Person* S_friends[1];
	A_friends[0] = Agent;
	Agent->friends = A_friends;
	Sonny->friends = S_friends;
	
	deleteP(Agent);
	deleteP(Sonny);
	return 0;
}
```

12. 
```c
Person* create(char* name, int age) {
	Person* p = (Person*) malloc(sizeof(Person));
	p->friends = malloc(sizeof(Person*) * 10);
	p->name = NULL;
	int i;
	for (i = 0; i < 10; i++) {
		p->friends[i] = NULL;
	}
	p->name = strdup(name);
	p->age = age;
	return p;
}
```

13. 
```c
void destroy(Person* p) {
	free(p->name);
	free(p->friends);
	memset(p, 0, sizeof(Person));
	free(p);
}
```

#Chapter5

1. 
getchar() can be used for getting characters from stdin and putchar() writing them to stdout.

2. 
When using gets(), there is no way to know how large the buffer is, which might lead to overflow and overwriting things that we don't want. 

3. 
```
int main(){
	char* input = "Hello 5 World";
	char hello[10];
	int five;
	char world[10];
	scanf(input, "%s %d %s", hello, &five, world);
	return 0;
}
```
4. 
#define _GNU_SOURCE
a buffer
capacity

5. 
```
#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
int main() {
	mode_t mode = S_IRUSR | S_IWUSR;
	int fildes = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
	write(fildes, "Hi! My name is Zhentian Zhang\nnextline\n", 39);
	close(fildes);
	
	FILE * file = fopen("hello_world.txt", "r");
	char *buffer = NULL;
	size_t capacity = 0;
	while (getline(&buffer, &capacity, file) != -1) {
		printf("%s", buffer);
	}
	return 0;
}
```

C Development

1. 
```bash
make debug
```

2. 
You will get a debug mode build instead of a new build. 

3. 
Tabs have to be used. 

4. 
It saves all the modification in the working tree. 
SHA is used with git revert to denote the commit version to revert.

5. 
It shows all the commits history. 

6. 
git status shows the state of the current working tree.
Things in gitignore will be ignored by git. 

7. 
It will update the remote repository. 
Commit is just a record, git push have to be used to update the changes. 

8. 
Git cannot update changes to the remote repository. 
We can use fetching and merging or pull the branch to fix it. 