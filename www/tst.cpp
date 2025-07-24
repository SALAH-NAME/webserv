#include "./include/ServerManager.hpp"

using namespace std;

void    printStr(std::string str, std::string request) {
	char lasChar;
	std::cout << "\n********************  " << str << "  *********************\n";
	for (size_t i = 0; i < request.size(); i++) {
		if (request[i] == '\r')
			std::cout << "\\r";
		else if (request[i] == '\n')
			std::cout << "\\n\n";
		else
			std::cout << request[i];
		lasChar = request[i];
	}
	if (lasChar != '\n')
		std::cout << "\n";
	std::cout << "********** " << str << " size: " << request.size() << " **************\n\n";
}

int main() {
    std::ifstream inputFile("inputFile.txt", std::ios::in | std::ios::binary);

    std::string holder;
    char buffer[1024];

    // while (getline(inputFile, buffer)) {
    //     holder += buffer;
    //     if (inputFile.peek() != EOF)  // More content follows → line ended with \n
    //     holder += '\n';
    // }

    int num_bytes = 1024;
    inputFile.read(buffer, num_bytes);
    buffer[inputFile.gcount()] = 0;

    cout << "file.gcount() ===>> " << inputFile.gcount() << "\n"; 

    printStr("BUFFER", string(buffer));
}