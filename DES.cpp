
#include "DES.h"

int main(int argc, char *argv[]) {
	unsigned char PlainText[8], Key[8];
	getKey(Key, argv[2]);
	int mode = (*argv[4] - 48);
	ifstream inFile(argv[1]);
	ofstream outFile(argv[3]);
	string PT;
	getline(inFile, PT);
	int i = 0;
	for (char c : PT) {
		PlainText[i] = c;
		i++;
		if (i == 8) {
			DES(Key, PlainText, &outFile, mode);
			i = 0;
			memset(PlainText, 32, 8);
		}
	}
	if (i != 0) { DES(Key, PlainText, &outFile, mode);}
	outFile.close();
	inFile.close();
	return 0;
}
void DES(unsigned char *Key, unsigned char *PlainText, ofstream *outFile, int mode) {
	unsigned char Kl[4], Kr[4], Kses[6], PTl[4], PTr[4], EPTr[6],SPTr[4];
	memset(Kl, 0, 4);
	memset(Kr, 0, 4);
	redcKey(Key);
	divKey(Key, Kl, Kr);
	iniText(PlainText);
	divPT(PlainText, PTl, PTr);
	for (int round = 0; round < 16; round++) {
		if (mode == 1) {
			shift(Kl, round, mode);
			shift(Kr, round, mode);
		}
		memset(Kses, 0, 6);
		memset(EPTr, 0, 6);
		memset(SPTr, 0, 4);
		genSessionKey(Kses, Kl, Kr);
		expPTR(EPTr, PTr);
		for (int i = 0; i < 6; i++) {
			EPTr[i] ^= Kses[i];
		}
		sboxPTR(SPTr, EPTr);
		for (int i = 0; i < 4; i++) {
			SPTr[i] ^= PTl[i];
		}
		for (int i = 0; i < 4; i++) {
			PTl[i] = PTr[i];
		}
		for (int i = 0; i < 4; i++) {
			PTr[i] = SPTr[i];
		}
		if (mode == 0) {
			shift(Kl, round, mode);
			shift(Kr, round, mode);
		}
	}
	for (int i = 0; i < 4; i++) {
		*(PlainText + i) = PTr[i];
	}
	for (int i = 0; i < 4; i++) {
		*(PlainText + (i + 4)) = PTl[i];
	}
	finText(PlainText);
	for (int i = 0; i < 8; i++) {
		*outFile << *(PlainText + i);
	}
}
void finText(unsigned char *PlainText) {
	unsigned char finPT[8], SB, SS;
	memset(finPT, 0, 8);
	for (int i = 0; i < 64; i++) {
		SS = FPER[i] - 1;
		SB = 128 >> (SS % 8);
		SB &= PlainText[SS / 8];
		SB <<= (SS % 8);
		finPT[i / 8] |= (SB >> (i % 8));
	}
	for (int i = 0; i < 8; i++) {
		*(PlainText + i) = finPT[i];
	}
}
void sboxPTR(unsigned char *SPTr, unsigned char *EPTr) {
	unsigned char SB, row, column, temp[8];
	memset(temp, 0, 8);
	for (int i = 0; i < 48; i++) {
		SB = 128 >> (i % 8);
		SB &= EPTr[i / 8];
		SB <<= (i % 8);
		temp[i / 6] |= (SB >> ((i % 6) + 2));
	}
	for (int i = 0; i < 8; i++) {
		row = 0; 
		column = 0;
		SB = 32;
		SB &= temp[i];
		row |= (SB >> 4);
		SB = 1;
		SB &= temp[i];
		row |= SB;
		for (int ii = 0; ii < 4; ii++) {
			SB = (16 >> ii);
			SB &= temp[i];
			column |= (SB >> 1);
		}
		*(SPTr + (i / 2)) |= *(SBOX[i] + (row * 16) + column);
		*(SPTr + (i / 2)) <<= (4 * ((i + 1) % 2));
	}
	funcPER(SPTr);
}
void funcPER(unsigned char *SPTr) {
	unsigned char func[4], SS, SB;
	memset(func, 0, 4);
	for (int i = 0; i < 32; i++) {
		SS = PPER[i] - 1;
		SB = 128 >> (SS % 8);
		SB &= SPTr[SS / 8];
		SB <<= (SS % 8);
		func[i / 8] |= (SB >> (i % 8));
	}
	for (int ii = 0; ii < 8; ii++) {
		*(SPTr + ii) = func[ii];
	}
}
void expPTR(unsigned char *EPTr, unsigned char *PTr) {
	unsigned char SS, SB;
	for (int i = 0; i < 48; i++) {
		SS = EPER[i] - 1;
		SB = 128 >> (SS % 8);
		SB &= PTr[SS / 8];
		SB <<= (SS % 8);
		*(EPTr + (i / 8)) |= (SB >> (i % 8));
	}
}
void divPT(unsigned char *PlainText, unsigned char *PTl, unsigned char *PTr) {
	for (int i = 0; i < 4; i++) {
		*(PTl + i) = PlainText[i];
	}
	for (int i = 0; i < 4; i++) {
		*(PTr + i) = PlainText[i + 4];
	}
}
void genSessionKey(unsigned char *Kses, unsigned char *Kl, unsigned char *Kr) {
	unsigned char SS, SB;
	for (int i = 0; i < 48; i++) {
		SS = CPER[i] - 1;
		if (SS >= 28) { 
			SB = 128 >> ((SS - 4) % 8);
			SB &= Kr[(SS - 28) / 8];
			SB <<= ((SS - 4) % 8);
		}
		else { 
			SB = 128 >> (SS % 8);
			SB &= Kl[SS / 8];
			SB <<= (SS % 8);
		}
		*(Kses + (i / 8)) |= (SB >> (i % 8));
	}
}
void shift(unsigned char *Key, int round, int mode) {
	unsigned char SBS, SB;
	if (mode == 1) {
		char shft = RNDS[round];
		for (int i = 0; i < shft; i++) {
			SBS = 0;
			SB = 0;
			for (int ii = 3; ii >= 0; ii--) {
				SBS = 128;
				SBS &= Key[ii];
				*(Key + ii) <<= 1;
				*(Key + ii) |= (SB >> 7);
				SB = SBS;
			}
			*(Key + 3) |= (SB >> 3);
		}
	}
	else if (mode == 0) {
		char shft = RNDS[15 - round];
		for (int i = 0; i < shft; i++) {
			SBS = 0;
			SB = 0;
			for (int ii = 0; ii < 3; ii++) {
				SBS = 1;
				SBS &= Key[ii];
				*(Key + ii) >>= 1;
				*(Key + ii) |= (SB << 7);
				SB = SBS;
			}
			SBS = 16;
			SBS &= Key[3];
			*(Key + 3) >>= 1;
			*(Key + 3) |= (SB << 7);
			*(Key + 3) &= 247;
			*Key |= (SBS << 3);
		}
	}
}
void divKey(unsigned char *Key, unsigned char *Kl, unsigned char *Kr) {
	unsigned char SB;
	for (int i = 0; i < 28; i++) {
		SB = 128 >> (i % 8);
		SB &= Key[i / 8];
		SB <<= (i % 8);
		*(Kl + (i / 8)) |= (SB >> (i % 8));
	}
	for (int i = 28; i < 56; i++) {
		SB = 128 >> (i % 8);
		SB &= Key[i / 8];
		SB <<= (i % 8);
		*(Kr + ((i - 28) / 8)) |= (SB >> ((i - 28) % 8));
	}
}
void redcKey(unsigned char *Key) {
	unsigned char redcKey[7], SB, SS;
	memset(redcKey, 0, 7);
	for (int i = 0; i < 56; i++) {
		SS = KPER[i] - 1;
		SB = 128 >> (SS % 8);
		SB &= Key[SS / 8];
		SB <<= (SS % 8);
		redcKey[i / 8] |= (SB >> (i % 8));
	}
	for (int ii = 0; ii < 7; ii++) {
		*(Key + ii) = redcKey[ii];
	}
	*(Key + 7) = 0;
}
void iniText(unsigned char *PlainText) {
	unsigned char iniPT[8], SB, SS;
	memset(iniPT, 0, 8);
	for (int i = 0; i < 64; i++) {
		SS = IPER[i] - 1;
		SB = 128 >> (SS % 8);
		SB &= PlainText[SS / 8];
		SB <<= (SS % 8);
		iniPT[i / 8] |= (SB >> (i % 8));
	}
	for (int i = 0; i < 8; i++) {
		*(PlainText + i) = iniPT[i];
	}
}
void getKey(unsigned char *Key, string _file) {
	ifstream File(_file);
	unsigned char c;
	for (int i = 0; i < 8; i++) {
		File >> c;		
		*(Key + i) = c;
	}
	File.close();
}