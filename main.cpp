#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

// 目录项
typedef struct directory_entry{
	char name[8];
	char extension[3];
	char attribute;
	char remain[10];
	char time[2];
	char date[2];
	unsigned short start_cluster;
	unsigned int size;
}DirEntry;

// 文件夹或文件项
typedef struct file_info {
	string name;
	string filter_name;
	int attribute;
	int start_cluster;
	int size;
	int level;
}FileInfo;

// 存取所有从a.img中拿出的表项
FileInfo array[1000];
int tableSize = 0;
string imgName = "a.img";

// asm函数
extern "C" {
	void asmPrint(const char *str, int color);
}

void ls(int pos, string paramter);
void cat(int pos);
void loop();
void excute(string command);

// 初始读取镜像函数，赋值给数组
int readImage();
// 读取文件夹下(包括子文件、子文件夹)
void readDir(FileInfo& dir, ifstream& image);

// 拿出文件属性（纯粹文件名、文件名包含路径）
void getFileName(FileInfo &fileInfo, DirEntry &dirEntry, string dirName);

// 返回数组索引
int findFile(string filePath);

// 获取FAT表项中下一簇
int getNextCluster(int cluster, ifstream& img);

// 计算子文件/夹数量
int subDirNum(int pos);
int subFileNum(int pos);

// 打印函数
void printStr(string s);
void printStr_Red(string s);

int main() {

	int control = readImage();
	if (control == 0){
        printStr("No such file or directory.\n");
        return;
    }

	loop();

	return 0;
}

void loop(){
    string command;
    while (true){
        printStr(">>");
        getline(cin, command);

        if (command=="exit") break;
        else{
            excute(command);
        }
    }
    return;
}

/**
 * 读取镜像，赋值给数组array
*/
int readImage(){
    ifstream image;
    image.open(imageName);

    if (!image){
        return 0;
    }
    FileInfo root;
    root.name = "/";
    root.attribute = 0;
    root.level = 0;
    root.size = 0;
    root.start_cluster = -12;
    array[tableSize++] = root;

    readDir(root, image);

    image.close();
    return 1;
}

/**
 * 读取当前文件夹下所有文件/夹
 * 并存放到数组内
*/
void readDir(FileInfo& dir, ifstream& image){
    DirEntry tempDirEntry;
    DirEntry* dir_ptr = &tempDirEntry;

    int start = (dir_ptr.start_cluster + 31) * 512;

    image.seekg(start);  
    for (int i=0; i<16; i++){
        image.seekg(start + 32 * i);  
        image.read((char *)dir_ptr, 32);

        char * name = tempDirEntry.name;

        if (name[0]=='\0') break;

        if (name[0]<'A' || name[0]>'Z') continue;
        else{
            if (tempDirEntry.attribute != 0x10 && tempDirEntry.attribute != 0x20){
                continue;
            }
            FileInfo fileInfo;
            getFileName(fileInfo, tempDirEntry, dir_ptr.name);
            if (tempDirEntry.attribute==0x10){
                fileInfo.attribute = 0;
            }
            else{
                fileInfo.attribute = 1;
            }
            fileInfo.start_cluster = tempDirEntry.start_cluster;
            fileInfo.size = tempDirEntry.size;
            fileInfo.level = dir_ptr.level + 1;

            array[tableSize] = fileInfo;
            tableSize++;

            // if this is a directory, recycle read Directory
            if (fileInfo.attribute == 0){
                readDir(fileInfo, image);
            }
        }
    }
}

/**
 * 获取纯粹的文件名,和包含路径的文件名
*/
void getFileName(FileInfo& fileInfo, DirEntry& dirEntry, string dirName){
    char name[20];
    int counter = 0;

    if (dirEntry.attribute == 0x10){
        while (true){
            if (counter == 8 || dirEntry.name[counter] == ' ') {
                break;
            }
            name[counter] = dirEntry.name[counter];
            counter++;
        }
    }else{
        while (true){
            // 文件名不满8字符以空格0x20填充
            if (counter == 8 || dirEntry.name[counter] == ' ') {
                break;
            }
            name[counter] = dirEntry.name[counter];
            counter++;
        }
        name[counter] = '.';
        counter++;
        for (int i=0; i<3; i++){
            name[counter + i] = dirEntry.extension[i];
        }
        counter += 3;
    }

    name[counter] = '\0';
    string str(name);
    fileInfo.filter_name = str;
    if (dirName == "/") {
        fileInfo.name = "/" + fileInfo.filter_name;
    }else{
        fileInfo.name = dirName + "/" + fileInfo.filter_name;
    }
}

void excute(string command){
    string mode;
    string paramter;
    string filePath;
    string temp;

    stringstream ss;
    ss << command;

    // 划分命令，如ls -l NJU, mode = "ls"
    ss >> mode;
    if (mode != "cat" && mode != "ls"){
        printStr("Command not found.\n");
        return;
    }

    // ss输入流内还剩-l NJU,则第一次循环temp = "-l", 第二次temp = "NJU"
    while (getline(ss, temp, ' ')){
        if (temp[0] == '-'){
            if (mode == "cat"){
                printStr("Command not found.\n");
                return;
            }
            else if (mode == "ls"){
                if (temp[1] != 'l'){
                    printStr("Command not found.\n");
                    return;
                }
                else{
                    paramter = "-l";
                }
            }else{
                return;
            }
        }
        else{
            if (filePath.length() == 0){
                filePath = temp;
            }
            else{
                printStr("No such file or directory.\n");
                return;
            }
        }
    }

    int pos = findFile(filePath);
    if (pos == -1){
        printStr("No such file or directory.\n");
        return;
    }

    if (mode == "ls"){
        if (array[pos].attribute != 0){
            printStr("It's not a directory.\n");
            return;
        }
        ls(pos, paramter);
    }

}

/*
*返回数组索引
**/
int findFile(string filePath){

    // 确保路径规范
    if (filePath.length() == 0 || filePath[0] != '/'){
        filePath = "/" + filePath;
    }
    if (filePath != "/"&&filePath[filePath.length() - 1] == '/') {
		filePath = filePath.substr(0, filePath.length() - 1);
	
    int pos = 0;
    while (true){
        if (pos >= tableSize){
            return -1;
        }
        if (array[pos].name == filePath){
            break;
        }
        pos++;
    }

    return pos;

}

void ls(int pos, string paramter){
    FileInfo& dirInfo = array[pos];
    if (dirInfo.level == 0){
        printStr(dir.name);
    }
    else{
        printStr(dir.name+"/");
    }

    // 打印子文件/夹数
    if (paramter == "-l") {
        printStr(" " + to_string(subDirNum(pos)));
		printStr(" " + to_string(subFileNum(pos)));
    }
    printStr(":\n");

    if (dir.level != 0){
        if (paramter == "-l"){
            printStr_Red(". ");
            printStr("\n");
            printStr_Red(".. ");
            printStr("\n");
        }else{
            printStr_Red(". .. ");
        }
    }

    pos++;

    // 子文件或子文件夹
    for (int i = pos; i < tableSize; i++) {
		FileInfo &curFile = array[i];
		if (curFile.level == dir.level + 1) {
			if (curFile.attribute == 0) {
				printStr_Red(curFile.filter_name+" ");
				if (paramter == "-l") {
					printStr(to_string(subDirNum(i)) + " ");
					printStr(to_string(subFileNum(i))+ "\n");
				}
			}
			else {
				printStr(curFile.filter_name + " ");
				if (paramter == "-l") {
					printStr(to_string(curFile.size) + "\n");
				}
			}
		}else{
            break;
        }
	}
	printStr("\n");

    // 子文件夹下的子文件和子文件夹，递归
    for (int i = pos; i < tableSize; i++) {
		FileInfo &curFile = array[i];
		if (curFile.level == dir.level + 1) {
			if (curFile.attribute == 0) {
				ls(i, paramter);
			}
		}else[
            break;
        ]
	}
}

void cat(int pos) {
	ifstream image;
	char text[10000];
	image.open(imgName);
	FileInfo &file = array[pos];
	string name = file.filter_name;

	if (name.length() < 4 || name.substr(name.length() - 4, 4) != ".TXT") {
		printStr("No such file.\n");
		return;
	}
	
	int cluster = file.start_cluster;
	int count = 0;
	while (true) {

		int address = (cluster + 31) * 512;
		image.seekg(address);
		image.read((char*)text + count*512, 512);//读取一个扇区的内容
		cluster = getNextCluster(cluster, image);
		if (cluster == -1) {
			break;
		}
		count++;
	}
	asmPrint(text, 0);
	printStr("\n");
	image.close();
}

int getNextCluster(int cluster, ifstream& image) {

    // 空白项
	if (cluster == 0) {
		return -1;
	}

	//读取第current个fat表项
	int n = cluster / 2 * 3;  // 将三个字节看作一组，且单位为byte,起始字节
	int type = cluster % 2;  // 判断左侧12bit还是右侧
	int num1 = 0;
	int num2 = 0;
	char *p1 = (char*)&num1;
	char *p2 = (char*)&num2;
	int next;
    // 代表左侧12bit
	if (type == 0) {
		image.seekg(1 * 512 + n);
		image.read(p1, 1);
		image.read(p2, 1);
		num2 = num2 & 0x0F;//取右四位，是低字节的一半
        // (8 7 6 5 4 3 2 1) + (12 11 10 9 0 0 0 0 0 0 0 0 0)
		next = num1 + (num2 << 8);
	}
	else {
        // 右侧12bit
		image.seekg(1 * 512 + n + 1);
		image.read(p1, 1);
		image.read(p2, 1);
		num1 = num1 & 0x00F0;//取4-7位
		next = (num1 >> 4) + (num2 << 4);
	}

	if (next >= 0x0FF8) {
		return -1;//已经是最后一个簇了
	}
	else {
		return next;
	}
}

//计算子目录个数
int subDirNum(int pos) {
	FileInfo &dir = array[pos];
	int num = 0;
	pos++;
	for (int i = pos; i < tableSize; i++) {
		FileInfo &curFile = array[i];
		if (curFile.level == dir.level) {
			break;
		}

		if (curFile.level == dir.level + 1 && curFile.attribute == 0) {
			num++;
		}
	}
	return num;
}

//计算子文件个数
int subFileNum(int pos) {
	FileInfo &dir = array[pos];
	int num = 0;
	pos++;
	for (int i = pos; i < tableSize; i++) {
		FileInfo &curFile = array[i];
		if (curFile.level == dir.level) {
			break;
		}

		if (curFile.level == dir.level + 1 && curFile.attribute == 1) {
			num++;
		}
	}
	return num;
}

// ptr指针占4byte, int也占4byte 
void printStr(string s) {
	const char *ptr = s.c_str();
	asmPrint(ptr, 0);
}

// ptr指针占4byte, int也占4byte 
void printStr_Red(string s) {
	const char *ptr = s.c_str();
	asmPrint(ptr, 1);
}