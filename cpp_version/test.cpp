#include <bits/stdc++.h>
#include "include/bencode.h"
using namespace std;

class A {
	public:
	long long x;
	long long getPriority() {
		return x;
	}

	void setPriority(long long x) {
		this->x = x;
	}
};

int main() {
    BENode be;  // make sure to delete the pointer if "be" is heap allocated pointer
    FILE * fp = fopen("tears-of-steel.torrent", "r");

    fseek(fp, 0, SEEK_END);
    int fileLen = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char s[fileLen+1];
    fread(s, 1, fileLen, fp);
    fclose(fp);

    BEDecode(s, &be);  // decoded and the tree's root is now "be"
    
    char *encoded;
    int encodedLen = BEEncode(&be, &encoded);  // bencodes the tree to "encoded"

    printf("file length : %d\nencoded length : %d\n", fileLen, encodedLen);
    for(int i=0;i<encodedLen;i++) {
        if(s[i] != encoded[i]) {
            printf("NO\nprefix match : %d\n", i);  // outputs dont match, prints matched prefix length
            
            delete encoded;
            return 0;
        }
    }
    printf("YES\n");  // outputs match

    delete encoded;  // clear heap allocated memory
}





