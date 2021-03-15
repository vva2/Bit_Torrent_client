#include "../include/bencode.h"

// BEList
BEList::BEList() {
    begin = NULL;
}

BEList::~BEList() {
    if(begin) delete begin;
}

BENode* BEList::operator[] (int index) {
    BEListNode* head = begin;
    while(head && index--) {
        head = head->next;
    }

    return head->value;
}

long long BEList::size() {
    BEListNode* head = begin;
    long long n = 0;
    while(head) {
        head = head->next;
        n++;
    }

    return n;
}


// BEListNode
BEListNode::BEListNode() {
    value = NULL;
    next = NULL;
}

BEListNode::~BEListNode() {
    // printf("BEListNode destructor\n");
    if(value) delete value;
    if(next) delete next;
}

// BEDict
BEDict::BEDict() {
    begin = NULL;
}

BEDict::~BEDict() {
    // printf("BEDict destructor\n");
    if(begin) delete begin;
}

BENode* BEDict::operator[] (const char key[]) {
    BEDictNode* head = begin;
    BENode* value = NULL;
    int n = strlen(key);
    while(head) {
        if(head->first->size == n && strncmp(head->first->value, key, n) == 0) {
            value = head->second;
        }
        head = head->next;
    }

    return value;
}


// BEDictNode
BEDictNode::BEDictNode() {
    first = NULL;
    second = NULL;
    next = NULL;
}

BEDictNode::~BEDictNode() {
    // printf("BEDictNode destructor\n");
    if(first) delete first;
    if(second) delete second;
    if(next) delete next;
}

// BEString
BEString::BEString() {
    value = NULL;
    size = 0;
}

BEString::~BEString() {
    // printf("BEString destructor\n");
    if(value) delete value;
}

// BENode
BENode::BENode() {
    obj = NULL;
}

void BENode::free() {
    this->~BENode();
}

BENode::~BENode() {
    // printf("BENode destructor\n");

    if(obj) {
        switch(type) {
            case BETypeDictionary:
                delete (BEDict*) obj;
                break;
            case BETypeInteger:
                delete (BEInt*) obj;
                break;
            case BETypeString:
                delete (BEString*) obj;
                break;
            case BETypeList:
                delete (BEList*) obj;
                break;
        }
    }
}

BENode* BENode::operator[] (const char key[]) {
    // works only for dictionary objects
    if(type != BETypeDictionary) return NULL;
    return (*((BEDict*) obj))[key];
}

BENode* BENode::operator[] (const int index) {
    // returns list node at index "index"
    // does not work for strings and dictionary
    if(type != BETypeList) return NULL;
    return (*((BEList*) obj))[index];
}

/*
    ////////////////
    * decoding part
    ////////////////
*/

void readBEInt(char *s, int *offset, BEInt *out) {
    out->value = 0;
    bool negative = false;

    if(s[0] == '-') {
        negative = true;
        (*offset)++;
    }

    while(s[*offset] != 'e') {
        (out->value) = (out->value)*10+(s[*offset]-'0');
        (*offset)++;
    }

    if(negative)
        out->value = -1*(out->value);
    
    (*offset)++;  // next item
}

void readBEString(char *s, int *offset, BEString *out) {
    // read length of string and copy the string
    out->size = 0;
    while(s[*offset] != ':') {
        out->size = (out->size)*10+(s[*offset]-'0');
        (*offset)++;
    }

    (*offset)++;  // skip ':' and point to head of the byte string
    if(out->size == 0) return;

    out->value = new char[out->size];
    memcpy(out->value, &s[*offset], out->size);
    (*offset) += out->size;
}

void readBEDict(char *s, int *offset, BEDict *out) {
    BEDictNode *head =  out->begin;
    while(s[*offset] != 'e') {
        if(head == NULL) {
            out->begin = new BEDictNode;
            head = out->begin;
        }
        else {
            head->next = new BEDictNode;
            head = head->next;
        }

        head->first = new BEString;
        readBEString(s, offset, head->first);
        head->second = new BENode;
        readBENode(s, offset, head->second);
    }

    (*offset)++;  // next item
}

void readBEList(char *s, int *offset, BEList *out) {
    BEListNode* head = out->begin;  // initially set to NULL

    while(s[*offset] != 'e') {
        if(head == NULL) {
            out->begin = new BEListNode;
            head = out->begin;
        }
        else {
            head->next = new BEListNode;
            head = head->next;
        }

        head->value = new BENode;
        readBENode(s, offset, head->value);
    }

    (*offset)++;  // next item
}

void readBENode(char *s, int *offset, BENode* out) {
    switch(s[*offset]) {
        case 'i':
            // read integer
            out->type = BETypeInteger;
            out->obj = new BEInt;
            (*offset)++;
            readBEInt(s, offset, (BEInt*) out->obj);
            break;
        case 'd':
            // read dictionary
            out->type = BETypeDictionary;
            out->obj = new BEDict;
            (*offset)++;
            readBEDict(s, offset, (BEDict*) out->obj);
            break;
        case 'l':
            // read list
            out->type = BETypeList;
            out->obj = new BEList;
            (*offset)++;
            readBEList(s, offset, (BEList*) out->obj);
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            // handle byte string
            out->type = BETypeString;
            out->obj = new BEString;
            readBEString(s, offset, (BEString*) out->obj);
            break;
        default:
            throw "error";
            break;
            // throw error
            // TODO : read how to throw exceptions
    }
}

void BEDecode(char *str, BENode* out) {
    // decodes the data into "out"
    // "out" variable can't be NULL here
    // does not accept empty string as input
    // start parsing the string recursively and build the tree
    int i = 0;  // offset
    
    if(str[0] != 'i' && str[0] != 'l' && str[0] != 'd' && str[0] < '0' && str[0] > '9') {
        // empty bencoded string
        return;
    }

    readBENode(str, &i, out);
}



/*
    ////////////////
    - encoding part
    ////////////////
*/

void writeInt(BE_INT x, int *len, char *out) {
    if(x < 0) {
        if(out) out[*len] = '-';
        (*len)++;
        x = -x;
    }

    int nSize = 0;

    while(x > 0) {
        if(out) out[(*len)+nSize] = (x%10)+'0';
        x = x/10;
        nSize++;
    }

    (*len) += nSize;

    if(!out) return;

    // reverse the integer
    int i = (*len)-nSize;
    int j = (*len)-1;
    char c;
    while(i < j) {
        // swap
        c = out[i];
        out[i] = out[j];
        out[j] = c;
        i++;
        j--;
    }
}

void encodeBEInt(BEInt *root, int *len, char *out) {
    if(out) out[*len] = 'i';
    (*len)++;

    writeInt(root->value, len, out);

    if(out) out[*len] = 'e';
    (*len)++;
}

void encodeBEString(BEString *root, int *len, char *out) {
    writeInt(root->size, len, out);
    if(out) out[*len] = ':';
    (*len)++;
    
    if(out) memcpy(&out[*len], root->value, root->size);
    (*len) += root->size;
}

void encodeBEList(BEList *root, int *len, char *out) {
    if(out) out[*len] = 'l';
    (*len)++;
    BEListNode* head = root->begin;
    while(head) {
        DFS(head->value, len, out);
        head = head->next;
    }
    if(out) out[*len] = 'e';
    (*len)++;
}

void encodeBEDict(BEDict *root, int *len, char *out) {
    if(out) out[*len] = 'd';
    (*len)++;
    BEDictNode* head = root->begin;
    while(head) {
        // read string
        encodeBEString(head->first, len, out);
        // read BENode
        DFS(head->second, len, out);
        head = head->next;
    }
    if(out) out[*len] = 'e';
    (*len)++;
}

void DFS(BENode* root, int *len, char *out) {
    if(root->type == BETypeString) {
        encodeBEString((BEString*) root->obj, len, out);
    }
    else if(root->type == BETypeInteger) {
        encodeBEInt((BEInt*) root->obj, len, out);
    }
    else if(root->type == BETypeList) {
        encodeBEList((BEList*) root->obj, len, out);
    }
    else {
        encodeBEDict((BEDict*) root->obj, len, out);
    }
}

int BEEncode(BENode* root, char **out) {
    // returns length of the decoded string
    // dumps the decoded data to "out"
    int len = 0;
    // get length first
    DFS(root, &len, NULL);
    // allocate heap memory
    *out = new char[len];
    // DFS to get the encoded string and final length
    len = 0;
    DFS(root, &len, *out);

    return len;
}