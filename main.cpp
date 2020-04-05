#include <iostream>
#include <fstream>
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <conio.h>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

using namespace std;
struct Adresy {
    string id,idZalogowanegoUzytkownika;
    string imie,nazwisko,nr_tel,email,adres;
};
struct Uzytkownicy {

    string  id;
    string login,haslo;
};
string konwersja_String(int liczba) {
    ostringstream ss;
    ss << liczba;
    string str = ss.str();
    return str;
}
int  OdczytKsiazki(vector <Adresy> &adresaci,string dana) {
    int id_Uzytkownika_int=0;
    int id_zalogowanego_Uzytkownika=1;
    string id_Uzytkownika="";
    Adresy osoby;
    fstream plik;
    plik.open("osoby.txt",ios::in);
    do {
        getline(plik,osoby.id,'|');
        getline(plik,osoby.idZalogowanegoUzytkownika,'|');
        getline(plik,osoby.imie,'|');
        getline(plik,osoby.nazwisko,'|');
        getline(plik,osoby.nr_tel,'|');
        getline(plik,osoby.email,'|');
        getline(plik,osoby.adres,'|');
        if(osoby.idZalogowanegoUzytkownika==dana) {
            adresaci.push_back(osoby);
        }
    if(osoby.id!="")
    {
       id_Uzytkownika=osoby.id;
       id_Uzytkownika_int=atoi(id_Uzytkownika.c_str());
        id_zalogowanego_Uzytkownika=id_Uzytkownika_int+1;
    }
    } while(getline(plik,osoby.id));
    plik.close();
    return id_zalogowanego_Uzytkownika;
}
int DodajOsoby( int id,string idZalogowanegoUzytkownika) {
    Adresy osoby;
    string imie="",nazwisko="",nr_tel="",email="",adres="";
    cout<< "Podaj imie: ";
    cin.sync();
    getline(cin,imie);
    cout<< "Podaj nazwisko: ";
    getline(cin,nazwisko);
    cout<< "Podaj numer telefonu: ";
    getline(cin,nr_tel);
    cout<< "Podaj email: ";
    cin.sync();
    getline(cin,email);
    cout<< "Podaj adres: ";
    cin.sync();
    getline(cin,adres);
    fstream plik;
    plik.open("osoby.txt",ios::out|ios::app);
    if (plik.good() == false) {
        plik.open("osoby.txt",std::ios::out);
    }
    plik<<id<<"|"<<idZalogowanegoUzytkownika<<"|"<<imie<<"|"<<nazwisko<<"|"<<nr_tel<<"|"<<email<<"|"<<adres<<"|"<<endl;
    plik.close();
    cout<< "Poprawnie dodano nowa osoby do ksiazki adresowej.";
}
void zapiszUzytkownikow(vector<Adresy> &adresaci) {
    fstream edycja;
    edycja.open("osoby.txt",ios::out|ios::trunc);
    for(int i=0; i<adresaci.size()-1; i++) {
        edycja<<adresaci[i].id<<"|"<<adresaci[i].imie<<"|"<<adresaci[i].nazwisko<<"|"<<adresaci[i].nr_tel<<"|"<<adresaci[i].email<<"|"<<adresaci[i].adres<<"|"<<endl;
    }
    edycja.close();
}
void WyswietlWektor(vector<Adresy> &adresaci,int j) {
    cout<<adresaci[j].id<<endl;
    cout<<adresaci[j].idZalogowanegoUzytkownika<<endl;
    cout<<adresaci[j].imie<<endl;
    cout<< adresaci[j].nazwisko<<endl;
    cout<< adresaci[j].nr_tel<<endl;
    cout<< adresaci[j].email<<endl;
    cout<< adresaci[j].adres<<endl;
    cout<<endl;
}
void WczytajKsiazke( vector<Adresy> &adresaci) {
    int    iloscUzytkownikowWksiazce= adresaci.size()-1;
    int i=0;
    for(int j=0; j<iloscUzytkownikowWksiazce; j++) {
        WyswietlWektor(adresaci,j);
        i++;
    }
    if(i==0) {
        cout<< "Brak uzytkownikow w ksiazce."<<endl;
    }
}
void WczytajWgNazwiska(vector<Adresy> &adresaci) {
    int   iloscUzytkownikowWksiazce= adresaci.size()-1;
    string nazwisko;
    cout<< "Podaj nazwisko: ";
    cin>>nazwisko;
    int q=0,i=0;
    while(q<iloscUzytkownikowWksiazce) {
        if(adresaci[q].nazwisko==nazwisko) {
            WyswietlWektor(adresaci,q);
            i++;
        }
        q++;
    }
    if(i==0) {
        cout<< "Uzytkownik o nazwisku: "<<nazwisko<< " nie istnieje w ksiazce adresowej."<<endl;
    }
}
void WczytajWgImienia(vector<Adresy> &adresaci) {
    int    iloscUzytkownikowWksiazce= adresaci.size()-1;
    string imie;
    cout<< "Podaj imie: ";
    cin>>imie;
    int q=0;
    int i=0;
    while(q<iloscUzytkownikowWksiazce) {
        if(adresaci[q].imie==imie) {
            WyswietlWektor(adresaci,q);
            i++;
        }
        q++;
    }
    if(i==0) {
        cout<< "Uzytkownik o imieniu: "<<imie<< " nie istnieje w ksiazce adresowej."<<endl;
    }
}
void zapisujeDoksiazkiPoEdycji(vector<Adresy> &adresaci) {
    Adresy osoby;
    int u=0;
    int j=0;
    fstream plik_tymczasowy;
    plik_tymczasowy.open("osoby.tymczasowy.txt",ios::out|ios::app);
    if (plik_tymczasowy.good() == false) {
        plik_tymczasowy.open("osoby.tymczasowy.txt",std::ios::out);
    }
    fstream plik;
    plik.open("osoby.txt",ios::in);
    do {
        getline(plik,osoby.id,'|');
        getline(plik,osoby.idZalogowanegoUzytkownika,'|');
        getline(plik,osoby.imie,'|');
        getline(plik,osoby.nazwisko,'|');
        getline(plik,osoby.nr_tel,'|');
        getline(plik,osoby.email,'|');
        getline(plik,osoby.adres,'|');
        vector<Adresy>::iterator wyraz = adresaci.begin()+j;
        if((wyraz->id==osoby.id)&&(osoby.id!="")) {
            plik_tymczasowy<<wyraz->id<<"|"<<wyraz->idZalogowanegoUzytkownika<<"|"<<wyraz->imie<<"|"<<wyraz->nazwisko<<"|"<<wyraz->nr_tel<<"|"<<wyraz->email<<"|"<<wyraz->adres<<"|"<<endl;
            j++;
        } else if(osoby.id!="") {
            plik_tymczasowy<<osoby.id<<"|"<<osoby.idZalogowanegoUzytkownika<<"|"<<osoby.imie<<"|"<<osoby.nazwisko<<"|"<<osoby.nr_tel<<"|"<<osoby.email<<"|"<<osoby.adres<<"|"<<endl;
        }
    }   while(getline(plik,osoby.id));
    plik.close();
    plik_tymczasowy.close();
    remove( "osoby.txt" );
    rename("osoby.tymczasowy.txt","osoby.txt");
    cout<< "Dane zostaly edytowane poprawnie";
}
vector<Adresy> zmieniaImie(vector<Adresy> &adresaci,string idUzytkownika) {
    string noweImie;
    for(int i=0; i<adresaci.size(); i++) {
        if(adresaci[i].id==idUzytkownika) {
            cout<< "Podaj nowe imie: ";
            cin>>noweImie;
            adresaci[i].imie=noweImie;
        }
    }
    zapisujeDoksiazkiPoEdycji(adresaci);
    return adresaci;
}
vector<Adresy> zmieniaNazwisko(vector<Adresy> &adresaci,string idUzytkownika) {
    string noweNazwisko;
    for(int i=0; i<adresaci.size(); i++) {
        if(adresaci[i].id==idUzytkownika) {
            cout<< "Podaj nowe nazwisko: ";
            cin>>noweNazwisko;
            adresaci[i].nazwisko=noweNazwisko;
        }
    }
    zapisujeDoksiazkiPoEdycji(adresaci);
    return adresaci;
}
vector<Adresy> zmieniaNumerTel(vector<Adresy> &adresaci,string idUzytkownika) {
    string noweNr_tel;
    for(int i=0; i<adresaci.size(); i++) {
        if(adresaci[i].id==idUzytkownika) {
            cout<< "Podaj nowe numer telefonu: ";
            cin>>noweNr_tel;
            adresaci[i].nr_tel=noweNr_tel;
        }
    }
    zapisujeDoksiazkiPoEdycji(adresaci);

    return adresaci;
}
vector<Adresy> zmieniaemail(vector<Adresy> adresaci,string idUzytkownika) {
    string noweemail;
    for(int i=0; i<adresaci.size(); i++) {
        if(adresaci[i].id==idUzytkownika) {
            cout<< "Podaj nowe email: ";
            cin>>noweemail;
            adresaci[i].email=noweemail;
        }
    }
    zapisujeDoksiazkiPoEdycji(adresaci);
    return adresaci;
}
vector<Adresy> zmieniaAdres(vector<Adresy> adresaci,string idUzytkownika) {
    string noweAdres;
    for(int i=0; i<adresaci.size(); i++) {
        if(adresaci[i].id==idUzytkownika) {
            cout<< "Podaj nowe adres: ";
            cin>>noweAdres;
            adresaci[i].adres=noweAdres;
        }
    }
    zapisujeDoksiazkiPoEdycji(adresaci);
    return adresaci;
}
vector <Adresy> usuwaUzytkownika( vector<Adresy> &adresaci,string idUsuwanegoUzytkownika) {
    for( vector<Adresy>::iterator i = adresaci.begin(); i<adresaci.end(); i++) {
        if(i->id == idUsuwanegoUzytkownika) {
            adresaci.erase(i);
            break;
        }
    }
    return adresaci;
}
void zapisujeDoksiazkiPoUsunieciuJesliUzytkownikIstnieje(vector<Adresy> &dosusuniecia,string idUsuwanegoUzytkownika) {
    fstream plik_tymczasowy;
    plik_tymczasowy.open("osoby.tymczasowy.txt",ios::out|ios::app);
    if (plik_tymczasowy.good() == false) {
        plik_tymczasowy.open("osoby.tymczasowy.txt",std::ios::out);
    }
    for(int j=0; j<dosusuniecia.size()-1; j++) {
        if(dosusuniecia[j].id!=idUsuwanegoUzytkownika) {
            string id,imie,nazwisko,nr_tel,email,adres,idZalogowanegoUzytkownika;
            id=dosusuniecia[j].id;
            idZalogowanegoUzytkownika=dosusuniecia[j].idZalogowanegoUzytkownika;
            imie=dosusuniecia[j].imie;
            nazwisko=dosusuniecia[j].nazwisko;
            nr_tel=dosusuniecia[j].nr_tel;
            email=dosusuniecia[j].email;
            adres=dosusuniecia[j].adres;
            plik_tymczasowy<<id<<"|"<<idZalogowanegoUzytkownika<<"|"<<imie<<"|"<<nazwisko<<"|"<<nr_tel<<"|"<<email<<"|"<<adres<<"|"<<endl;
        }
    }
    plik_tymczasowy.close();
    remove( "osoby.txt" );
    rename("osoby.tymczasowy.txt","osoby.txt");
    cout<< "Dane zostaly edytowane poprawnie";
}
void zapisujeDoksiazkiPoUsunieciuJesliUzytkownikNieIstnieje(vector<Adresy> &dosusuniecia) {
    fstream plik_tymczasowy;
    plik_tymczasowy.open("osoby.tymczasowy.txt",ios::out|ios::app);
    if (plik_tymczasowy.good() == false) {
        plik_tymczasowy.open("osoby.tymczasowy.txt",std::ios::out);
    }
    for(int j=0; j<dosusuniecia.size()-1; j++)  {
        string id,imie,nazwisko,nr_tel,email,adres,idZalogowanegoUzytkownika;
        id=dosusuniecia[j].id;
        idZalogowanegoUzytkownika=dosusuniecia[j].idZalogowanegoUzytkownika;
        imie=dosusuniecia[j].imie;
        nazwisko=dosusuniecia[j].nazwisko;
        nr_tel=dosusuniecia[j].nr_tel;
        email=dosusuniecia[j].email;
        adres=dosusuniecia[j].adres;
        plik_tymczasowy<<id<<"|"<<idZalogowanegoUzytkownika<<"|"<<imie<<"|"<<nazwisko<<"|"<<nr_tel<<"|"<<email<<"|"<<adres<<"|"<<endl;
    }
    plik_tymczasowy.close();
    remove( "osoby.txt" );
    rename("osoby.tymczasowy.txt","osoby.txt");
    cout<<"Nie ma takiego uytkownika";
}
void zapisujeDoksiazkiPoUsunieciu(vector<Adresy> &adresaci,string idUsuwanegoUzytkownika,string idZalogowanegoUzytkownika) {
    vector<Adresy> dosusuniecia;
    Adresy osoby;
    int u=0;
    int j=0;
    fstream plik;
    plik.open("osoby.txt",ios::in);
    do {
        getline(plik,osoby.id,'|');
        getline(plik,osoby.idZalogowanegoUzytkownika,'|');
        getline(plik,osoby.imie,'|');
        getline(plik,osoby.nazwisko,'|');
        getline(plik,osoby.nr_tel,'|');
        getline(plik,osoby.email,'|');
        getline(plik,osoby.adres,'|');
        dosusuniecia.push_back(osoby);
    } while(getline(plik,osoby.id));
    plik.close();
    int czypodanyUzytkownikIstniejeWbazie=0;
    for(int l=0; l<adresaci.size(); l++) {
        if(adresaci[l].id==idUsuwanegoUzytkownika) {
            czypodanyUzytkownikIstniejeWbazie++;
            break;
        }
    }
    if(czypodanyUzytkownikIstniejeWbazie==1)  {
        zapisujeDoksiazkiPoUsunieciuJesliUzytkownikIstnieje(dosusuniecia,idUsuwanegoUzytkownika);
    }
    if(czypodanyUzytkownikIstniejeWbazie==0) {
        zapisujeDoksiazkiPoUsunieciuJesliUzytkownikNieIstnieje(dosusuniecia);
    }
}
int  czytaBazeUzytkownikow(vector <Uzytkownicy> &uzytkownik) {
    int iloscOsobWplku=0;
    Uzytkownicy uzytk;
    fstream plik;
    plik.open("Uzytkownicy.txt",ios::in);
    do {
        getline(plik,uzytk.id,'|');
        getline(plik,uzytk.login,'|');
        getline(plik,uzytk.haslo,'|');
        uzytkownik.push_back(uzytk);
        iloscOsobWplku++;
    } while(getline(plik,uzytk.id));
    plik.close();
    return iloscOsobWplku-1;
}
void zapiszUzytkownikow(vector<Uzytkownicy> &adresaci) {
    fstream edycja;
    edycja.open("Uzytkownicy.txt",ios::out|ios::trunc);
    for(int i=0; i<adresaci.size()-1; i++) {
        edycja<<adresaci[i].id<<"|"<<adresaci[i].login<<"|"<<adresaci[i].haslo<<"|"<<endl;
    }
    edycja.close();
}
void rejestracja( vector <Uzytkownicy> &uzytkownik) {
    string nazwa, haslo;
    cout<< "Podaj nazwe uzytkwnika: ";
    cin>>nazwa;
    int i=0;
    while( i<uzytkownik.size()) {
        if(uzytkownik[i].login==nazwa) {
            cout<<"Uzytkowniek o nazwie: "<<uzytkownik[i].login<<" juZ istnieje. Wpisz inna nazwe uzytkwnika: ";
            cin>>nazwa;
            i=0;
        } else {
            i++;
        }
    }
    cout<< "Podaj haslo: ";
    cin>>haslo;
    int iloscUzytkownikow=uzytkownik.size();
    int id_int=iloscUzytkownikow;
    string id_string=konwersja_String(id_int);
    fstream plik;
    plik.open("Uzytkownicy.txt",ios::out|ios::app);
    if (plik.good() == false) {
        plik.open("Uzytkownicy.txt",std::ios::out);
    }
    plik<<id_string<<"|"<<nazwa<<"|"<<haslo<<"|"<<endl;
    plik.close();
    cout<< "Konto zalozone!"<<endl;
    Sleep(1000);
}
string logowanie(vector <Uzytkownicy> &uzytkownik) {
    string nazwa;
    string haslo;
    vector<Adresy> adresaci;
    int i=0;
    int d=0;
    cout<< "Podaj nazwe uzytkownika: ";
    cin>>nazwa;
    while(i<uzytkownik.size()) {
            d++;
        if(uzytkownik[i].login == nazwa) {
            for(int q=0; q<3; q++) {
                cout<< "Podaj haslo. Pozostalo "<<3-q<< " prob"<<": ";
                cin>>haslo;
                if(uzytkownik[i].haslo==haslo) {
                    return uzytkownik[i].id;
                }
            }
             return "";
        }
        i++;
    }
    if(d==(uzytkownik.size()))
    {
        return "";
    }
}
void  zmianahasla(vector <Uzytkownicy> &uzytkownik,string idZalogowanegoUzytkownika) {
    string nowehaslo;
    cout<< "Podaj nowe haslo: ";
    cin>>nowehaslo;
    for( int i=0; i<uzytkownik.size()-1; i++) {
        if(uzytkownik[i].id==idZalogowanegoUzytkownika) {
            uzytkownik[i].haslo=nowehaslo;
            cout<< "Haslo zostalo zmienione"<<endl;
            Sleep(1500);
        }
        zapiszUzytkownikow(uzytkownik);
    }
}
int main() {
    char wybor;
    char ktoremenu='1';
    string idZalogowanegoUzytkownika="";
    while(true) {
        vector <Uzytkownicy> uzytkownik;
        czytaBazeUzytkownikow(uzytkownik);
        int ilosc;
        ilosc =uzytkownik.size()-1;
        if(ktoremenu=='1') {
            system("cls");
            cout<< "Ksiazka adresowa"<<endl;
            cout<<"------------------"<<endl;
            cout<< "1.Rejestracja"<<endl;
            cout<< "2.Logowanie"<<endl;
            cout<< "3.Zamknij program"<<endl;
            cin>>wybor;
            if(wybor=='1') {
                system("cls");
                rejestracja(uzytkownik);
            } else if(wybor=='2') {
                system("cls");
                if(uzytkownik.size()==1)
                {
                    cout <<"Brak uzytkownikow";
                    Sleep(2000);
                }
                else{
                idZalogowanegoUzytkownika=logowanie(uzytkownik);
                if(idZalogowanegoUzytkownika=="") {
                    cout<< "Login lub haslo niepoprawne. Sprobuj raz jeszcze.";
                    Sleep(3000);
                    ktoremenu='1';
                } else if(idZalogowanegoUzytkownika!="") {
                    ktoremenu='2';
                }
                }
            } else if(wybor=='3') {
                exit(0);
            }
        }
        if(ktoremenu=='2') {
            Sleep(3000);
            system("cls");
            cout<< "1.Dodaj adresata "<<endl;
            cout<< "2.Wyswietl wszystkich adresatow"<<endl;
            cout<< "3.Wyszukaj po nazwisku "<<endl;
            cout<< "4.Wyszukaj po imieniu "<<endl;
            cout<< "5.Edytuj adresata "<<endl;
            cout<< "6.Usun adresata"<<endl;
            cout<< "7.Zmien haslo"<<endl;
            cout<< "9.Wyloguj sie"<<endl;
            cin>>wybor;
            ;
            vector<Adresy> adresaci;
            int id_Uzytkownika=OdczytKsiazki(adresaci,idZalogowanegoUzytkownika);
            if(wybor=='1') {
                system("cls");
                DodajOsoby(id_Uzytkownika,idZalogowanegoUzytkownika);
            } else if(wybor=='2')  {
                system("cls");
                WczytajKsiazke(adresaci);
                cout<< "Aby przejsc do Menu glownego nacisnij ENTER.";
                getchar();
                getchar();
            } else if(wybor=='3') {
                system("cls");
                WczytajWgNazwiska(adresaci);
                cout<< "Aby przejsc do Menu glownego nacisnij ENTER.";
                getchar();
                getchar();
            } else if(wybor=='4') {
                system("cls");
                WczytajWgImienia(adresaci);
                cout<< "Aby przejsc do Menu glownego nacisnij ENTER.";
                getchar();
                getchar();
            } else if(wybor=='5') {
                system("cls");
                string idUzytkownika;
                char danaDoZmiany;
                cout<< "Podaj id  uzytkownika ktorego chcesz edytowac: ";
                cin>>idUzytkownika;
                int decyzja=0;
                for(int l=0; l<adresaci.size(); l++) {
                    if(adresaci[l].id==idUzytkownika) {
                        decyzja++;
                        break;
                    }
                }
                if(decyzja==1) {
                    system("cls");
                    cout<< "Chce zmienic: "<<endl;
                    cout<< "1-Imie "<<endl;
                    cout<< "2-Nazwisko "<<endl;
                    cout<< "3-Numer telefony "<<endl;
                    cout<< "4-Adres email"<<endl;
                    cout<< "5-Adres"<<endl;
                    cin>>danaDoZmiany;
                    system("cls");
                    if(danaDoZmiany=='1') {
                        zmieniaImie(adresaci,idUzytkownika);
                    } else if(danaDoZmiany=='2') {
                        zmieniaNazwisko(adresaci,idUzytkownika);
                    } else if(danaDoZmiany=='3') {
                        zmieniaNumerTel(adresaci,idUzytkownika);
                    } else if(danaDoZmiany=='4') {
                        zmieniaemail(adresaci,idUzytkownika);
                    } else if(danaDoZmiany=='5') {
                        zmieniaAdres(adresaci,idUzytkownika);
                    }
                 }else {
                    cout<< "Nie ma takiego uzytkownika w ksiazce";
                }
            } else if(wybor=='6') {
                system("cls");
                cout<< "Id uzytkownika ktorego chcesz usunac: ";
                string id_usun;
                cin>>id_usun;
                cout<< "Czy napewno chcesz usunac uzutkownika o id: "<<id_usun<< " ?(t/n)";
                char decyzjaCzyusunacUzytkownika;
                cin>>decyzjaCzyusunacUzytkownika;
                if(decyzjaCzyusunacUzytkownika=='t') {
                    zapisujeDoksiazkiPoUsunieciu(adresaci,id_usun,idZalogowanegoUzytkownika);
                    usuwaUzytkownika(adresaci,id_usun);
                }
            } else if(wybor=='7') {
                system("cls");
                zmianahasla(uzytkownik,idZalogowanegoUzytkownika);
            } else if(wybor=='9') {
                ktoremenu='1';
            }
        }
    }
    return 0;
}
