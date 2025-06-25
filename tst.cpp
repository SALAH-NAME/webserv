#include <netinet/in.h> // For sockaddr_in
#include <unistd.h>     // For close()
#include <iostream>
#include <string>
#include <string.h>
#include <arpa/inet.h> // for inet_addr()
#include <vector>

using namespace std;

class TST {
    public:
        TST(int i) {
            if (i == 0) {
                cout << "return\n";
                return ;
            }
        }
};

int main() {
    vector<TST> v;

    v.push_back(TST(1));

    std::cout << "size ==> " << v.size() << "\n";
}