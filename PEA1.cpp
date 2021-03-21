//PEA Etap 1 [BF + DP] Jakub Pawleniak 248897
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <chrono>
#include <stdlib.h>
#include <time.h>


using namespace std;


class TSP {
public:
    int size = 0;        //ilosc wierzcholkow
    int** tab = NULL;   //macierz kosztow
    int pathLength = 0; //do przechowywania biezacej dlugosci sciezki
    int minPathLength = 0; //aktualna minimalna dlugosc sciezki
    int cityptr = 0, cityhptr = 0; //pointery na stosy ciezek
    int v0 = 0;         //wierzcholek poczatkowy
    bool* visited = NULL;   //lista odwiedzonych miast
    int* cityList = NULL;   //tablica do przechowywania sciezki
    int* cityListh = NULL;  //pomocnicza tablica do przechowywania sciezki
    int d2;

    int VISITED_ALL = 0;     // maska pelnej sciezki (zapelniana odpowiednią iloscia jedynek po wczytaniu algorytmu DP)
    int** dptab = NULL;        //tablica z maskami i wartoscami odwiedzonych juz sciezek
    int** dptab2 = NULL;        //tablica do tworzenia sciezki powrotnej (backtracing)

    int seed = time(NULL);      //seed do losowania instancji

    
    ~TSP() {
      /*  if (dptab != NULL) {
            for (int i = 0; i < d2 -1; i++) {
                delete[] dptab[i];
            }
            delete[]dptab;
        }
        if (dptab2 != NULL) {
            for (int i = 0; i < d2; i++) {
                delete[] dptab2[i];
            }
            delete[]dptab2;
        }
        
        if (tab != NULL) {
            for (int i = 0; i < size; i++) {
                delete[] tab[i];
            }
            delete[]tab;
        }
        */
    }
    
    
    void clearAll() {        //reset atrybutów
        pathLength = 0;
        minPathLength = 0;
        v0 = 0;
        cityhptr = cityptr = 0;

        VISITED_ALL = 0;
        
        

    }

    void showTSP() {	//	wyświetlenie macierzy kosztów
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                cout << tab[i][j] << "    ";
            }
            cout << endl;
        }
    }

    void saveTSP() {                //zapisanie poprzedniej instancji
        string path = "last.txt";
        ofstream ofile(path);
        if (ofile)
        {
            ofile << size << endl;
            for (int o = 0;o < size;o++)
            {
                for (int oo = 0;oo < size;oo++)
                {
                    ofile << tab[o][oo] << " ";
                }
                ofile << endl;
            }
        }
        else {
            cout << "Blad !: problem z zapisaniem." << endl;
        }
        ofile.close();
        ofile.clear();
    }

    void randTSP(int n) {           //tworzenie losowej instancji
        srand(seed);
        seed = rand();
        size = n;
        tab = new int* [size];
        for (int i = 0; i < size; i++) {
            tab[i] = new int[size];
        }
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (i == j) tab[i][j] = 0;
                else {
                    tab[i][j] = rand() % 100;
                }
            }
        }

        clearAll();
    }
    void testAL(int c1, int c2, int alg) {          // funkcja zlóżąca do testów potrzebnych do sprawozdania
        string path;
        if (alg == 1) path = "testBF.txt";
        else if (alg == 2) path = "testDP.txt";

        ofstream ofile(path);
        if (ofile)
        {
            ofile << c1 << endl;
            ofile << endl;
            for (c2; c2 > 0; c2--) {
                randTSP(c1);
                if (alg == 1)
                {
                    auto start = chrono::steady_clock::now();
                    bruteForce();
                    auto stop = chrono::steady_clock::now();
                    auto time = stop - start;
                    ofile << time / chrono::milliseconds(1) << endl;
                }
                if (alg == 2)
                {
                    auto start = chrono::steady_clock::now();
                    dpStart();
                    auto stop = chrono::steady_clock::now();
                    auto time = stop - start;
                    ofile << time / chrono::milliseconds(1) << endl;
                }

                

            }
            ofile.close();
            ofile.clear();

        }
        else {
            cout << "Blad !: problem z zapisaniem pliku testu BF." << endl;
        }

    }

    bool loadTSP(string& name) {	//wczytanie grafu z pliku, inicjacja potrzebnych zmiennych, macierzy oraz tabel
        fstream file;
        string line;
        file.open(name, ios::in);
        if (!file.good()) {
            cout << "\nBlad wczytywania pliku.\n";
            file.close();
            file.clear();
            return false;
        }
        else {
            getline(file, line);
            size = stoi(line);
            cout << "\nIlosc wierzcholkow: " << size << "\n";

            tab = new int* [size];
            for (int i = 0; i < size; i++) {
                tab[i] = new int[size];
            }
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    file >> tab[i][j];
                }
            }
        }


        clearAll();

        file.close();
        file.clear();
        return true;
    }

    void bruteForce() {	//	rozpoczęcie algorytmu brute force
        
        visited = new bool[size];
        cityList = new int[size];
        cityListh = new int[size];
        
        clearAll();

        for (int i = 0; i < size; i++) {
            visited[i] = false;
        }

        v0 = 0;

        TSPbf(v0);                          //wywołanie rekurencji dla wierzcholka poczatkowego

        cout << "Znaleziona sciezka: ";
        for (int i = 0; i < cityptr; i++) {
            cout << cityList[i] << " -> ";
        }
        cout << v0 << endl;
        cout << "Minimalna dlugosc sciezki: " << minPathLength << endl;

        delete[] cityList;
        delete[] cityListh;
        delete[] visited;
        clearTab();


    }

    void TSPbf(int v) {	// algorytm rekurencyjny do brute force'a
        int u;

        cityListh[cityhptr++] = v;              //wpisanie wierzcholka na liste pomocniczą

        if (cityhptr < size) {                  //jezeli droga nie jest kompletna
            visited[v] = true;                  
            for (u = 0; u < size; u++) {            //sprawdzaj kolejne wierzchołki
                if (visited[u] == false)
                {
                    pathLength += tab[v][u];
                    TSPbf(u);                       
                    pathLength -= tab[v][u];
                }
            }
            visited[v] = false;
        }
        else {                                  //jezeli droga jest kompletna
            pathLength += tab[v][v0];           //dodaj koszt powrotu
            if (pathLength < minPathLength || minPathLength < 1) {      //i sprawdz czy łącznie jest mniejszy niż poprzednio sprawdzany
                minPathLength = pathLength;                             //jezeli tak przypisz jej wartosc oraz trase 
                for (u = 0; u < cityhptr; u++) {
                    cityList[u] = cityListh[u];
                }
                cityptr = cityhptr;
            }
            pathLength -= tab[v][v0];
        }
        cityhptr--;
    }
    // =========================================Programowanie dynamiczne
    void dpStart() {            
        cityList = new int[size];
        clearAll();
        d2 = pow(2, size);
        v0 = 0;
        VISITED_ALL = d2 - 1;

        dptab = new int* [d2];                                      
        dptab2 = new int* [d2];
        for (int i = 0; i < d2; i++) {
            dptab[i] = new int[size];
            dptab2[i] = new int[size];

        }

        for (int i = 0;i < d2;i++) {                                    //uzupelnienie tablic -1
            for (int j = 0;j < size;j++) {
                dptab[i][j] = -1;
                dptab2[i][j] = -1;
            }
        }

        minPathLength = dp(1, 0);
        cityList[cityptr++] = 0;
        dpPath(1, 0);                                                  //wywolanie funkcji dla (maski poczatkowej ..001 oraz wierzchołka 0
        

        for (int i = 0; i < size; i++) {
            cout << cityList[i] << "->";
        }
        cout << "0" << endl;

        delete[]cityList; 
        
       
        for (int z = 0;z < d2;z++) {                                        //zwalnianie pamięci
            delete[]dptab[z];
            delete[]dptab2[z];
        }
        delete[]dptab;
        delete[]dptab2;
        clearTab();




    }

    void clearTab() {
        if (tab != NULL) {
            for (int i = 0; i < size; i++) {
                delete[] tab[i];
            }
            delete[]tab;
        }
    }

    int dp(int mask, int pos)
    {
        if (mask == VISITED_ALL)     // wszystkie wierzchołki odwiedzone
        {
            return tab[pos][0];
        }
        if (dptab[mask][pos] != -1)  // zabezpieczenie przed kolejnym obliczaniem juz wyliczonej wartosci dla podproblemu
        {
            return dptab[mask][pos];
        }

        int ans = INT_MAX;

        for (int v = 0; v < size; v++)
        {
            if ((mask & (1 << v)) == 0) {                           //jezeli operacja AND przekazanej maski i maski wierzchołka == 0 to wierzchołka nie ma jeszcze na trasie
                int newAns = tab[pos][v] + dp(mask | (1 << v), v);      //suma kosztu trasy z poprzedniego wierzcholka do rozpatrywanego + rozwiazanie kolejnego podproblemu
                //ans = min(ans, newAns);
                if (newAns < ans)
                {
                    ans = newAns;                                   //jezeli lepsza to przypisz
                    dptab2[mask][pos] = v;
                }
            }
        }
        dptab[mask][pos] = ans;
        return ans;
    }
    void dpPath(int mask, int pos) {                          //backtracing - 'odkrywanie' sciezki z minimalnym kosztem
        if (dptab2[mask][pos] == -1)
        {
            return;                                             //jezeli -1 to zakończ bo koniec trasy
        }
        int i = dptab2[mask][pos];                              //pobranie kolejnego wierzchołka
        cityList[cityptr++] = i;
        //cout<< i << "->";
        int newSubset = mask | (1 << i);                        //modyfikacja maski operacja OR "dodaje" pobrany wierzchołek do maski 
        dpPath(newSubset, i);
    }


};

int main() {

    TSP tspobj = TSP();
    int choice, choice2;
    string filename;
    bool dataCtrl = false;

    do {
        cout << "PEA Etap 1 Jakub Pawleniak 248897\n-------------------------------\n"
            << "Wybierz opcje: \n"
            << "1.Wczytaj dane\n"
            << "2.Generuj dane\n"
            << "3.Wczytaj poprzednie dane\n"
            << "9.Testy do sprawozdania\n"
            << "0.Zakoncz\n";
        cin >> choice;

        switch (choice) {
        case 1: {
            cout << "Podaj nazwe pliku z grafem: " << endl;
            cin >> filename;
            cout << endl;
            if (tspobj.loadTSP(filename))
            {
                char yesno;
                cout << "Wyswietlic macierz? [y/n]" << endl;
                cin >> yesno;
                if (yesno == 'y' || yesno == 'Y') {
                    tspobj.showTSP();
                }
                else {
                    cout << "Koniec wczytywania" << endl;
                }
                dataCtrl = true;
            }
            break;}
        case 2: {
            int inst;
            cout << "Podaj wielkosc instancji: " << endl;
            cin >> inst;
            cout << endl;
            tspobj.randTSP(inst);
            tspobj.showTSP();
            dataCtrl = true;
            break;

        case 3: {
            string pa = "last.txt";
            if (tspobj.loadTSP(pa))
            {
                char yesno;
                cout << "Wyswietlic macierz? [y/n]" << endl;
                cin >> yesno;
                if (yesno == 'y' || yesno == 'Y') {
                    tspobj.showTSP();
                }
                else {
                    cout << "Koniec wczytywania" << endl;
                }
                dataCtrl = true;
            }
            break;
        }
        }
        case 9: {
            dataCtrl = true;
            break;}
        case 0: {
            break;}

        default:
        {
            cout << "Bledny wybor zrodla danych" << endl;
        }
        }

        if((dataCtrl != false) & (choice!=0)){
       
        cout << "PEA Etap 1 Jakub Pawleniak 248897\n-------------------------------\n"
            << "Wybierz opcje: \n"
            << "1.Brute Force\n"
            << "2.Dynamic Programing\n"
            << "0.Zakoncz\n";

        cin >> choice2;
        switch (choice2) {
        case 1: {
            if (choice == 9)
            {
                int c1, c2;
                cout << "Wielkosc generowanych instancji:" << endl;
                cin >> c1;
                cout << "Ilosc probek" << endl;
                cin >> c2;
                tspobj.testAL(c1, c2, choice2);
            }
            else {
                tspobj.saveTSP();
                auto start = chrono::steady_clock::now();
                tspobj.bruteForce();
                auto stop = chrono::steady_clock::now();
                auto elapsed_seconds = stop - start;
                cout << tspobj.minPathLength << endl;
                cout << elapsed_seconds / chrono::milliseconds(1) << endl;
            }
            break;
        }
        case 2: {
            if (choice == 9)
            {
                int c1, c2;
                cout << "Wielkosc generowanych instancji:" << endl;
                cin >> c1;
                cout << "Ilosc probek" << endl;
                cin >> c2;
                tspobj.testAL(c1, c2, choice2);
            }
            else {
                auto start = chrono::steady_clock::now();
                tspobj.dpStart();
                auto stop = chrono::steady_clock::now();
                auto elapsed_seconds = stop - start;
                cout << tspobj.minPathLength << endl;
                cout << endl;
                cout << "Czas wykonania: " << elapsed_seconds / chrono::milliseconds(1) << " ms." << endl;
                cout << "Czas wykonania: " << elapsed_seconds.count() << " ns." << endl;
            }
            break;
        }
        case 0: {
            choice = 0;
            break;
        }

        default:
        {
            cout << "Bledny wybor" << endl;
            break;
        }
        }
        }
       
    }while (choice != 0);

    return 0;
}