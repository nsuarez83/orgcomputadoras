#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#define TAMANIO_BLOQUE "tamanioBloque"
#define TAMANIO_CACHE "tamanioCache"
#define CANT_VIAS "cantidadVias"
#define CACHEGRIND  "valgrind --tool=cachegrind --cachegrind-out-file=salidaCachegrind "
#define CG_ANNOTATE "cg_annotate --show=Dr,Dw,D1mr,D1mw salidaCachegrind "
#define FILE_OUTPUT_CACHEGRIND "salidaCachegrind"
#define FILE_OUTPUT_CGANNOTATE "salidaCgannotate"
#define FILE_OUTPUT_GREP "salidaGrep"
#define SIZE_BUFFER 512

using std::string;

void replaceText(string &word, const string &toReplace, const string &replaceBy)
{

	string::size_type posToRepleace = word.find(toReplace);
	while(posToRepleace != string::npos)
	{
		string tmpWord = word;
		if (posToRepleace > 0)
		{
			word = tmpWord.substr(0, posToRepleace);
			word += replaceBy;
			word += tmpWord.substr(posToRepleace + toReplace.size(), tmpWord.size() - 1);
		}
		else
		{
			word = replaceBy;
			word += tmpWord.substr(posToRepleace + toReplace.size(), tmpWord.size() - 1);
		}
		posToRepleace = word.find(toReplace);
	}
}

string intToString( int entero )
{
    std::stringstream cadena("");
    cadena << entero;
    return cadena.str();
}

void parsearDatos(const string &funcion, long int &dr, long int &dw, long int &d1mr, long int &d1mw)
{
	string grep("grep " + funcion + " " + FILE_OUTPUT_CGANNOTATE + " > " + FILE_OUTPUT_GREP);
	system(grep.c_str());
	std::ifstream fileGrep;
	fileGrep.open(FILE_OUTPUT_GREP);
	if (!fileGrep.good())
	{
		std::cerr <<  "Error de lectura de archivo temporal" << std::endl;
		return;
	}
	char buffer[SIZE_BUFFER];
	fileGrep.getline(buffer, SIZE_BUFFER);
	string word(buffer);

	int posDesde = 0, posHasta = 0;
	posDesde = word.find_first_not_of(" ", posDesde);
	posHasta = word.find(" ", posDesde);
	string strDr = word.substr(posDesde, posHasta-posDesde);
	replaceText(strDr, ",", "");
	dr = atol(strDr.c_str());

	posDesde = word.find_first_not_of(" ", posHasta);
	posHasta = word.find(" ", posDesde);
	string strDw = word.substr(posDesde, posHasta-posDesde);
	replaceText(strDw, ",", "");
	dw = atol(strDw.c_str());

	posDesde = word.find_first_not_of(" ", posHasta);
	posDesde = word.find(" ", posDesde);
	string strD1mr = word.substr(posDesde, posHasta-posDesde);
	replaceText(strD1mr, ",", "");
	d1mr = atol(strD1mr.c_str());

	posDesde = word.find_first_not_of(" ", posHasta);
	posDesde = word.find(" ", posDesde);
	string strD1mw = word.substr(posDesde, posHasta-posDesde);
	replaceText(strD1mw, ",", "");
	d1mw = atol(strD1mw.c_str());
	fileGrep.close();
}

int tamanioBloque(char *datosCache)
{
	string cachegrind(CACHEGRIND);
	cachegrind.append(datosCache);
	cachegrind.append(" ./");
	cachegrind.append(TAMANIO_BLOQUE);
	system(cachegrind.c_str());

	string cgannotate(CG_ANNOTATE);
	cgannotate.append(TAMANIO_BLOQUE);
	cgannotate.append(".cpp");
	cgannotate.append(" > ");
	cgannotate.append(FILE_OUTPUT_CGANNOTATE);
	system(cgannotate.c_str());

	long int dr=0, dw=0, d1mr=0, d1mw=0;
	string fileFuente(TAMANIO_BLOQUE);
	fileFuente.append(".cpp:");
	fileFuente.append(TAMANIO_BLOQUE);
	parsearDatos(fileFuente, dr, dw, d1mr, d1mw);
	int sizeBloque = 0;
	if(dw>0  && d1mw>0)
		sizeBloque = dw/d1mw;

	remove(FILE_OUTPUT_CACHEGRIND);
	remove(FILE_OUTPUT_CGANNOTATE);
	remove(FILE_OUTPUT_GREP);
	return sizeBloque;
}

int tamanioCache(char *datosCache, int tamanioBloque)
{
	int n = 128;
	long int dr=0, dw=0, d1mr=0, d1mw=0;
	while(d1mw < n)
	{
		n = n*2;
		string cachegrind(CACHEGRIND);
		cachegrind.append(datosCache);
		cachegrind.append(" ./");
		cachegrind.append(TAMANIO_CACHE);
		cachegrind.append(" " + intToString(tamanioBloque) + " " + intToString(n));
		system(cachegrind.c_str());

		string cgannotate(CG_ANNOTATE);
		cgannotate.append(TAMANIO_CACHE);
		cgannotate.append(".cpp");
		cgannotate.append(" > ");
		cgannotate.append(FILE_OUTPUT_CGANNOTATE);
		system(cgannotate.c_str());

		string fileFuente(TAMANIO_CACHE);
		fileFuente.append(".cpp:");
		fileFuente.append(TAMANIO_CACHE);
		parsearDatos(fileFuente, dr, dw, d1mr, d1mw);

		remove(FILE_OUTPUT_CACHEGRIND);
		remove(FILE_OUTPUT_CGANNOTATE);
		remove(FILE_OUTPUT_GREP);
	}
	if(n==512)
		n *= 2;
	return (n/2 * tamanioBloque);
}

int main(int argc, char* argv[])
{
	if(argc==1)
		return 1;
	int sizeBloque = 0, sizeCache = 0;
	sizeBloque = tamanioBloque(argv[1]);
	sizeCache = tamanioCache(argv[1], sizeBloque);
	std::cout << "Tamaño de bloque: " << sizeBloque << " Bytes"<<std::endl;
	std::cout << "Tamaño de cache: " << sizeCache << " Bytes"<<std::endl;
	return 0;
}
