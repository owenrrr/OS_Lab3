#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

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


typedef struct file_info {
	string name;
	string filter_name;
	int attribute;
	int start_cluster;
	int size;
	int level;
}FileInfo;

FileInfo fileInfoArray[1000];
int tableSize = 0;
string imgName = "a.img";

extern "C" {
	void asmPrint(const char *str, int color);
}
void printStr(string s);
void printStr_Red(string s);
void excute(string commond);
void loop();
int readImage();
void readDir(FileInfo& dir, ifstream& image);
void getFileName(FileInfo &fileInfo, DirEntry &dirEntry, string dirName);
void ls(int pos, string paramter);
void cat(int pos);
int subDirNum(int pos);
int subFileNum(int pos);
int findFile(string filePath);
int getNextCluster(int cluster, ifstream& img);

int main() {

	int control = readImage();
	if (control == 0){
        printStr("Cannot find such file or directory.\n");
        return;
    }

	loop();

	return 0;
}

void loop(){
    string commond;
    while (true){
        printStr(">>");
        getline(cin, commond);

        if (commond=="exit") break;
        else{
            excute(commond);
        }
    }
    return;
}

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
    fileInfoArray[tableSize++] = root;

    readDir(root, image);

    image.close();
    return 1;
}

void readDir(FileInfo& dir, ifstream& image){
    DirEntry tempDirEntry;
    DirEntry* dir_ptr = &tempDirEntry;

    int start = (dir.start_cluster + 31) * 512;

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
            getFileName(fileInfo, tempDirEntry, dir.name);
            if (tempDirEntry.attribute==0x10){
                fileInfo.attribute = 0;
            }
            else{
                fileInfo.attribute = 1;
            }
            fileInfo.start_cluster = tempDirEntry.start_cluster;
            fileInfo.size = tempDirEntry.size;
            fileInfo.level = dir.level + 1;

            fileInfoArray[tableSize] = fileInfo;
            tableSize++;

            // if this is a directory, recycle read Directory
            if (fileInfo.attribute == 0){
                readDir(fileInfo, image);
            }
        }
    }
}

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

void printStr(string s) {
	const char *ptr = s.c_str();
	asmPrint(ptr, 0);
}

void printStr_Red(string s) {
	const char *ptr = s.c_str();
	asmPrint(ptr, 1);
}

void excute(string commond){
    string mode;
    string paramter;
    string filePath;
    string temp;

    stringstream ss;
    ss << commond;

    ss >> mode;
    if (mode != "cat" && mode != "ls"){
        printStr("Commond not found.\n");
        return;
    }

    while (getline(ss, temp, ' ')){
        if (temp[0] == '-'){
            if (mode == "cat"){
                printStr("Commond not found.\n");
                return;
            }
            else if (mode == "ls"){
                if (temp[1] != 'l'){
                    printStr("Commond not found.\n");
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
        if (fileInfoArray[pos].attribute != 0){
            printStr("It's not a directory.\n");
            return;
        }
        ls(pos, paramter);
    }

}

int findFile(string filePath){

    if (filePath.length() == 0 || filePath[0] != '/'){
        filePath = "/" + filePath;
    }
    if (filePath != "/"&&filePath[filePath.length() - 1] == '/') {
		filePath = filePath.substr(0, filePath.length() - 1);
	}

    int pos = 0;
    while (true){
        if (pos >= tableSize){
            return -1;
        }
        if (fileInfoArray[pos].name == filePath){
            break;
        }
        pos++;
    }

    return pos;

}

void ls(int pos, string paramter){
    FileInfo& dirInfo = fileInfoArray[pos];
    if (dirInfo.level == 0){
        printStr(dir.name);
    }
    else{
        printStr(dir.name+"/");
    }
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

    // Sub file or directory
    for (int i = pos; i < tableSize; i++) {
		FileInfo &curFile = fileInfoArray[i];
		if (curFile.level == dir.level) {
			break;
		}

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
		}
	}
	printStr("\n");

    // under sub directory
    for (int i = pos; i < tableSize; i++) {
		FileInfo &curFile = fileInfoArray[i];
		if (curFile.level == dir.level) {
			break;
		}

		if (curFile.level == dir.level + 1) {
			if (curFile.attribute == 0) {
				ls(i, paramter);
			}
		}
	}
}

void cat(int pos) {
	ifstream image;
	char text[10000];
	image.open(imgName);
	FileInfo &file = fileInfoArray[pos];
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
	int n = cluster / 2 * 3;  //得到奇偶合并的3字节的起始字节位置
	int type = cluster % 2;
	int num1 = 0;
	int num2 = 0;
	char *p1 = (char*)&num1;
	char *p2 = (char*)&num2;
	int next;
	if (type == 0) {
		//若是偶数，处理左侧两字节
		image.seekg(1 * 512 + n);
		image.read(p1, 1);
		image.read(p2, 1);
		num2 = num2 & 0x0F;//取右四位，是低字节的一半
        // (8 7 6 5 4 3 2 1) + (12 11 10 9 0 0 0 0 0 0 0 0 0)
		next = num1 + (num2 << 8);
	}
	else {
		//若是奇数，处理右侧两字节
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
	FileInfo &dir = fileInfoArray[pos];
	int num = 0;
	pos++;
	for (int i = pos; i < tableSize; i++) {
		FileInfo &curFile = fileInfoArray[i];
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
	FileInfo &dir = fileInfoArray[pos];
	int num = 0;
	pos++;
	for (int i = pos; i < tableSize; i++) {
		FileInfo &curFile = fileInfoArray[i];
		if (curFile.level == dir.level) {
			break;
		}

		if (curFile.level == dir.level + 1 && curFile.attribute == 1) {
			num++;
		}
	}
	return num;
}

