#ifndef _MY_FILE_H_
#define _MY_FILE_H_
#include <vector>
#include <iostream>
#include <map>
#include <string>

using namespace std;

struct  my_file {
	char *data;
	int fh;
	string file_name;
};

struct my_dir {
	vector<my_file> file_list;
	vector<my_dir> dir_list;
	string dir_name;
	int dh;
};
/*
 * We need to have an init function
 * that puts the root inside the file system
 * making the first directory in the hierachy
 */
my_dir root = {
	.dir_name = "/",
	.dh = 0
};

map<int, my_file> file_table;

map<string, my_dir> dir_table;
dir_table[root.dh] = root;

vector<my_file> LOOF;

#endif
