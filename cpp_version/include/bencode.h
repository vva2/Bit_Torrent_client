#pragma once
// #include <stdio.h>  // for debugging: printf
#include <string.h>  // used "memcpy" and "NULL"
#define BE_INT long long  // specified in the specs


// info source : https://wiki.theory.org/index.php/BitTorrentSpecification#Bencoding
// TODO: can add functionality for reading directly from a file: i.e stream variable as input
// TODO add functionality to pretty print the BENode
// replace new with malloc for C-lang compatibility

// TODO write usage here
///////////////////////////////////////////////////////////////////////////////////////////////////////
/*
#include "bencode.h"
#include <stdio.h>

int main() {
    BENode be;  // make sure to delete the pointer if "be" is heap allocated pointer
    FILE * fp = fopen("puppy.torrent", "r");

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
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////
enum BEType {
    None,
    BETypeInteger,
    BETypeString,
    BETypeList,
    BETypeDictionary
};

struct BEInt {
    BE_INT value;
};

struct BEString {
    int size;
    char *value;

    BEString();
    ~BEString();
};

struct BENode {
    BEType type;
    void *obj;  // declared void* instead of union to avoid circular dependancy and delete errors

    BENode();
    void free();
    ~BENode();
    BENode* operator[] (const char key[]);
    BENode* operator[] (int index);
};

struct BEDictNode {
    BEString *first;
    BENode *second;
    BEDictNode* next;

    BEDictNode();
    ~BEDictNode();
};

struct BEDict {
    BEDictNode* begin;

    BEDict();
    ~BEDict();
    // indexing
    BENode* operator[] (const char key[]);
};


struct BEListNode {
    BENode* value;
    BEListNode* next;

    BEListNode();
    ~BEListNode();
};

struct BEList {
    BEListNode* begin;

    BEList();
    ~BEList();
    BENode* operator[] (int index);
    long long size();
};

// functions
void readBEInt(char *s, int *offset, BEInt *out);
void readBEString(char *s, int *offset, BEString *out);
void readBEDict(char *s, int *offset, BEDict *out);
void readBEList(char *s, int *offset, BEList *out);
void readBENode(char *s, int *offset, BENode* out);
void BEDecode(char *str, BENode* out);

void writeInt(BE_INT x, int *len, char *out);
void encodeBEInt(BEInt *root, int *len, char *out);
void encodeBEString(BEString *root, int *len, char *out);
void encodeBEList(BEList *root, int *len, char *out);
void encodeBEDict(BEDict *root, int *len, char *out);
void DFS(BENode* root, int *len, char *out);
int BEEncode(BENode* root, char **out);




// printing
// void printBENode(BENode* root, int indent=0);

// void printIndent(int indent) {
//     for(int i=0;i<indent;i++)
//         printf(" ");
// }

// void printBEDict(BEDict *root, int indent=0) {
//     printIndent(indent);
//     printf("[\n");
//     BEDictNode *head = root->begin;

//     while(head) {
//         printBENode(head->value, indent+4);
//         printf(",\n");
//         head = head->next;
//     }
//     printIndent(indent);
//     printf("]\n");
// }

// void printBEList(BEList *root, int indent=0) {
//     printIndent(indent);
//     printf("[\n");
//     BEListNode *head = root->begin;

//     while(head) {
//         printBENode(head->value, indent+4);
//         printf(",\n");
//         head = head->next;
//     }
//     printIndent(indent);
//     printf("]\n");
// }

// void printBENode(BENode* root, int indent=0) {
//     // indent : increments by four
//     if(root->type == BETypeDictionary)
//         printBEDict(root->d, indent);
//     else if(root->type == BETypeInteger) {
//         for(int i=0;i<indent;i++)
//             printf(" ");
//         printf("%lld", root->i->value);
//     }
//     else if(root->type == BETypeList)
//         printBEList(root->l, indent);
//     else if(root->type == BETypeString) {
//         for(int i=0;i<indent;i++)
//             printf(" ");
//         fwrite(root->s->value, root->s->size, 1, stdout);
//     }
// }