#include <fstream>
#include <map>
#include <sstream>
#include <sys/stat.h>

std::ofstream resources_cpp, resources_h;

void load(std::string file);
void write(std::string file, int size, char *data);


const char * love_header = ""
"/**\n"
"* Copyright (c) 2006-2009 LOVE Development Team\n"
"* \n"
"* This software is provided 'as-is', without any express or implied\n"
"* warranty.  In no event will the authors be held liable for any damages\n"
"* arising from the use of this software.\n"
"* \n"
"* Permission is granted to anyone to use this software for any purpose,\n"
"* including commercial applications, and to alter it and redistribute it\n"
"* freely, subject to the following restrictions:\n"
"* \n"
"* 1. The origin of this software must not be misrepresented; you must not\n"
"*    claim that you wrote the original software. If you use this software\n"
"*    in a product, an acknowledgment in the product documentation would be\n"
"*    appreciated but is not required.\n"
"* 2. Altered source versions must be plainly marked as such, and must not be\n"
"*    misrepresented as being the original software.\n"
"* 3. This notice may not be removed or altered from any source distribution.\n"
"**/\n\n";



using namespace std;

int main(int argc, char *argv[])
{
	if (argc > 1)
	{
		resources_h.open("resources.h");
		resources_h
			<< love_header
			<< "#ifndef LOVE_RESOURCES_H\n"
			<< "#define LOVE_RESOURCES_H\n\n"
			<< "namespace love\n"
			<< "{\n";
		resources_cpp.open("resources.cpp");
		resources_cpp
			<< love_header
			<< "#include \"resources.h\"\n\n"
			<< "namespace love\n"
			<< "{\n";
		for (int i = 1; i < argc; i++)
		{
			// use boost::filesystem to list all the files (in v2 maybe)
			load(argv[i]);
		}
		resources_cpp << "};\n";
		resources_cpp << "} // love\n";
		resources_cpp.close();
		resources_h
			<< "}\n\n"
			<< "#endif // LOVE_RESOURCES_H \n";
		resources_h.close();
	}

	else
		printf("ResHax-5Million v1.0a\n- now empowered by rubber piggies\n\nUsage: reshax-5million [file1] [file2] [and so on...]\n");
}

void load(string file)
{
	ifstream fs (file.c_str(), fstream::in | fstream::binary | fstream::ate);
	if (fs.is_open() && fs.good())
	{
		printf("Haxing %s", file.c_str());
		
		int size = (int)fs.tellg();
		char * buff = new char[size];
		fs.seekg(0, ios::beg);
		fs.read(buff, size);
#ifndef WIN32
		struct stat fstat;
		stat(file.c_str(), &fstat);
		if (!S_ISDIR(fstat.st_mode))
			write(file, size, buff);
		else
			printf(" FAIL cuz DIR\n");
#else
		write(file, size, buff);
#endif
		fs.close();
		delete [] buff;
	}
	else
		printf("Hax does not liek '%s'\n", file.c_str());
}

void write(string file, int size, char *data)
{
	char buffer[8];
	//size--;

	// removes directory from file path
	size_t found;
	found = file.find_last_of('/');
	if (found != string::npos)
		file = file.substr(found + 1);

	string var (file);
	// replaces dashes and dots (in UNIX only)
	//#ifndef WIN32
	found = var.find_first_of(" !\"#$%&'()*+,-.@[]", 0);
	while (found != string::npos)
	{
		var[found] = '_';
		found = var.find_first_of(" !\"#$%&'()*+,-.@[]", found);
	}
	//#endif

	resources_cpp << "\tconst char " << var << "_data[] = {";
	for (int i = 0; i < size - 1; i++)
	{
		sprintf(buffer, "%d,", data[i]);
		resources_cpp << buffer;
		if (i != 0 && i % 30 == 0)
			resources_cpp << "\n\t";
	}
	sprintf(buffer, "%d};\n", data[size]);

	resources_h << "\t// " << var << "\n";
	resources_h << "\tconst int " << var << "_size = " << size << ";\n";
	resources_h << "\textern const char " << var << "_data[];\n\n";
	printf(" is haxed\n");
}
