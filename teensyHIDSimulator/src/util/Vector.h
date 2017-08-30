
typedef float scalar;

template<unsigned int N>
void vecAdd(scalar* res,const scalar* a,const scalar* b) {
	for(unsigned int i = 0; i<N; i++) {
		res[i] = a[i] + b[i];
	}
}

template<unsigned int N>
void vecSub(scalar* res,const scalar* a,const scalar* b) {
	for(unsigned int i = 0; i<N; i++) {
		res[i] = a[i] - b[i];
	}
}

template<unsigned int N>
void vecMult(scalar* res, const scalar& a, const scalar* vec) {
	for(unsigned int i = 0; i<N; i++) {
		res[i] = a*vec[i];
	}
}

template<unsigned int N>
void vecMult(scalar* res, const scalar* a, const scalar* b) {
	for(unsigned int i = 0; i<N; i++) {
		res[i] = a[i]*b[i];
	}
}
