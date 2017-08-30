template<typename T>
unsigned int typeToBuffer(char* buffer,const T& type, unsigned int start=0, unsigned int typeLength=1) {
    char* p = (char *)((void *)(&type));
    buffer += start;
    unsigned int typeSize = sizeof(T)*typeLength;
    unsigned int i;
    for(i=0; i<typeSize; i++) {
        *buffer++ = *p++;
    }
    return i;
}

template<typename T>
unsigned int bufferToType(const char* buffer, T& type, unsigned int start=0, unsigned int typeLength=1) {
    char* p = (char *)((void *)(&type));
    buffer += start;
    unsigned int typeSize = sizeof(T)*typeLength;
    unsigned int i;
    for(i=0; i<typeSize; i++) {
        *p++ = *buffer++;
    }
    return i;
}
