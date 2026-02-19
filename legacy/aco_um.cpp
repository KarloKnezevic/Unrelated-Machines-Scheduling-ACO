/*  

 _____              _____  _____    ____   _____   ______  _____  _____ __      __       _   _       _  ______ 
|  __ \     /\     / ____||  __ \  / __ \ |  __ \ |  ____||  __ \|_   _|\ \    / //\    | \ | |     | ||  ____|
| |__) |   /  \   | (___  | |__) || |  | || |__) || |__   | |  | | | |   \ \  / //  \   |  \| |     | || |__   
|  _  /   / /\ \   \___ \ |  ___/ | |  | ||  _  / |  __|  | |  | | | |    \ \/ // /\ \  | . ` | _   | ||  __|  
| | \ \  / ____ \  ____) || |     | |__| || | \ \ | |____ | |__| |_| |_    \  // ____ \ | |\  || |__| || |____ 
|_|  \_\/_/    \_\|_____/ |_|      \____/ |_|  \_\|______||_____/|_____|    \//_/    \_\|_| \_| \____/ |______|
                                                                                                               
 _    _ 
| |  | |
| |  | |
| |  | |
| |__| |
 \____/ 
        
  ____   _  __ _____   _    _  ______     _  _    _ 
 / __ \ | |/ /|  __ \ | |  | ||___  /    | || |  | |
| |  | || ' / | |__) || |  | |   / /     | || |  | |
| |  | ||  <  |  _  / | |  | |  / /  _   | || |  | |
| |__| || . \ | | \ \ | |__| | / /__| |__| || |__| |
 \____/ |_|\_\|_|  \_\ \____/ /_____|\____/  \____/ 
                                                    
 _   _  ______   _____  _____    ____   _____   _   _  _____  _    _ 
| \ | ||  ____| / ____||  __ \  / __ \ |  __ \ | \ | ||_   _|| |  | |
|  \| || |__   | (___  | |__) || |  | || |  | ||  \| |  | |  | |__| |
| . ` ||  __|   \___ \ |  _  / | |  | || |  | || . ` |  | |  |  __  |
| |\  || |____  ____) || | \ \ | |__| || |__| || |\  | _| |_ | |  | |
|_| \_||______||_____/ |_|  \_\ \____/ |_____/ |_| \_||_____||_|  |_|
                                                                     
                                                                     
  _____  _______  _____    ____        _  ______ __      __      
 / ____||__   __||  __ \  / __ \      | ||  ____|\ \    / //\    
| (___     | |   | |__) || |  | |     | || |__    \ \  / //  \   
 \___ \    | |   |  _  / | |  | | _   | ||  __|    \ \/ // /\ \  
 ____) |   | |   | | \ \ | |__| || |__| || |____    \  // ____ \ 
|_____/    |_|   |_|  \_\ \____/  \____/ |______|    \//_/    \_\
                                                                 

+-++-++-++-++-++-++-+ +-++-++-+
|Z||A||V||R||S||N||I| |R||A||D|
+-++-++-++-++-++-++-+ +-++-++-+

+-++-++-++-++-+ +-++-++-++-++-++-++-++-++-+ +-++-++-++-++-++-++-++-++-++-+
|K||A||R||L||O| |K||N||E||Z||E||V||I||C||,| |0||0||3||6||4||4||3||5||6||8|
+-++-++-++-++-+ +-++-++-++-++-++-++-++-++-+ +-++-++-++-++-++-++-++-++-++-+

Mentor: doc.dr.sc. Domagoj Jakobović

*/

#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <cmath>

//vrijednost beskonacne vrijednosti
#define Beskonacnost 32767

using namespace std;

/********************************************GLOBALNE VARIJABLE********************************************/

//datoteke procitane iz GLAVNE ULAZNE DATOTEKE, fitness.txt
string dolasci_poslova="";	//fitness_ready.txt
string trajanja_poslova="";	//fitness_duration.txt
string zavrsetci_poslova="";	//fitness_deadline.txt
string tezinski_faktori_poslova="";	//fitness_weight.txt

//podaci o broju poslova i strojeva
vector<int> vektor_poslova;
vector<int> vektor_strojeva;

//podaci iz GLAVNE ULAZNE DATOTEKE, fitness.txt
//informacije iz glave ulazne datoteke
typedef struct {

	int broj_skupova;		//broj skupova
	int max_br_poslova;		//maksimalan broj poslova u skupu
	int max_br_strojeva;	//maksimalan broj strojeva u skupu		
	int max_trajanje_posla;	//najdulje trajanje posla
	
}ulazna_informacija;

ulazna_informacija konstante;

//struktura podataka POSAO
typedef struct{

	int brposla;					//redni broj posla

	vector<int> trajanje;			//vektor trajanja izvrsavanja na svakom stroju; index i je index stroja
	int Deadline;					//vrijeme željenog završetka
	int Rpripravnost;				//vrijeme pripravnosti
	double Wtezina;					//težinski faktor
	
	int Czavrsetak;					//zavrsetak obrade posla
	int Fprotjecanje;				//F=C-R
	int Lkasnjenje;					//L=C-D
	int Tzaostajanje;				//T=max(0,L)
	int Uzakasnjelost;				//U={1 : T>0	,	0 : T<=0}

	int rb_stroj;					//redni broj stroja koji obrađuje posao
	int prioritet;					//prioritet u obradi na stroju
	int prethodnik;					//posao koji je na obradi prije this posla
	int prethodnik_povezi;			//za ACO na zahtjev

	
	//metoda za racunanje kasnjenja posla u obradi
	void kasnjenje(){
		Lkasnjenje=Czavrsetak-Deadline;
	}

	//metoda za racunanje koliko je vremena posao proveo u sustavu
	void protjecanje(){
		Fprotjecanje=Czavrsetak-Rpripravnost;
	}

	//metoda za racunanje zaostajanja u sustavu, a ako kasnjenja nije bilo, zaostajanje je 0
	void zaostajanje(){

		if(Lkasnjenje>0){
			Tzaostajanje=Lkasnjenje;
		}else
			Tzaostajanje=0;

	}

	//bool metoda za oznaku je li obrada nekog posla kasnila ili nije
	void zakasnjelost(){

		if(Tzaostajanje>0){
			Uzakasnjelost=1;
		}else{
			Uzakasnjelost=0;
		}

	}

} posao;

//vektor poslova
vector<posao> poslovi;

//izračunati podaci za skup poslova (4. informacije o kvaliteti rasporeda)
typedef struct{
	int Cnaj;
	double Fnaj;
	double Tnaj;
	double Unaj;
		
}evaluacija;

evaluacija dobrota_rasporeda;


//varijable za ACO

//broj mrava
int broj_mrava;
//broj iteracija ACO algoritma
int broj_generacija_mrava;

//inicijalna vrijednost feromonskog traga
double tau;
//vrijednost alfa parametra (naglasak na odabiru pomoću feromonskog traga)
double alfa;
//vrijednost beta parametra (naglasak na odabitu pomoću heurističke funkcije)
double beta;
//vrijednost parametra za isparavanje feromonskog traga (govori koliki dio traga NE isparava)
double ro;

/*zbroj trajanja i resursa sljedece aktivnosti
3D matrica									*/		
vector<vector<vector<double> > > heuristicke_vrijednosti;
/*vrijednost feromonskog traga za sve aktivnosti
2D matrica									*/
vector<vector<vector<double> > > jacina_feromonskog_traga;
/*lista posjecenih aktivnosti pojedniog mrava
Inicijalna vrijednost je 1, a nakon posjeta postaje 0
2D matrica									*/
vector<vector<int> > tabu_lista;

//pocetna aktivnost mrava
vector<vector<int> > pocetna_aktivnost_mrava;
//trenutna aktivnost mrava
vector<vector<int> > trenutna_aktivnost_mrava;
/*redoslijed obilaska aktivnosti
2D matrica									*/
vector<vector<vector<int> > > redoslijed_obilaska;
//sljedeca aktivnost
vector<vector<int> > sljedeca_aktivnost;	
vector<vector<int> > najbolji_slijed_aktivnosti;


/*vrijednost heurističkog parametra za sve aktivnosti
2D matrica; potreban parametar je varijabla heuristicke_vrijednosti
1/(suma trajanja i resursa sljedece aktivnosti)	*/
vector<vector<vector<double> > > tablica_heuristickih_vrijednosti; 
//trajanje projekta za svakog mrava
vector<double> trajanje_projekta;
//vektor za praćenje prioriteta na stroju prilikom ACO raspoređivanja na zahtjev 
vector<int> prioritet_na_stroju_zahtjev;
vector<int> job_prethodi;

int tip_rasporedivanja=0;
int kriterij_optimizacije=0;
int pocetni_index, zavrsni_index;
double koef_mravi;
string ime_izlaz_dat;
string ime_dominacija_dat;

typedef struct{

	double Cduljina;
	double Fprotjecanje;
	double Tzaostajanje;
	double Uzakasnjelost;

}dominacija;

dominacija dominacija_podaci;

/*pseudoslučajni parametar Q
Q je potreban da bi mrav odlučio hoće li odlazak u novu aktivnost biti donesena na temelju vjerojatnosne funkcije ("istraživanje")
ili će primjeniti stečeno znanje i otići u onu aktivnost čiji je produkt jacina_feromonskog_traga i tablica_heuristickih_vrijednosti 
najveci.
Ako je q0>=q, primjenjuje znanje, u suprotnom promatra vjerojatnosnu funkciju*/
double q;
//vrijednost q0 je e[0,1], a vrlo često e[0.8,0.9]
double q0=0.8;
ofstream izlazRAS("trajanje_rasporedivanja.txt");	
/********************************************GLOBALNE VARIJABLE********************************************/
/********************************************METODE ZA ACO********************************************/

void inicijalizacija_poslova(int N){
	
	vector<double> pom1;
	vector<vector<double> > pom2;

	//inicijalno resetiranje vrijednosti
	heuristicke_vrijednosti.clear();
	pocetna_aktivnost_mrava.clear();
	tablica_heuristickih_vrijednosti.clear();


	//unos inicijalnih vrijednosti potrebnih za heuristiku
	//koristi se funkcija <double sumiraj>
	for(int i=0;i<vektor_poslova[N];i++){
		for(int j=0;j<vektor_poslova[N];j++){
			for(int k=0;k<vektor_strojeva[N];k++){
				pom1.push_back(poslovi[j].trajanje[k]);
			}
			pom2.push_back(pom1);
			pom1.clear();
		}
		heuristicke_vrijednosti.push_back(pom2);
		pom2.clear();
	}

	//postavlja se da je heuristika za istu aktivnost beskonačna
	for(int i=0;i<vektor_poslova[N];i++){
		for(int j=0;j<vektor_poslova[N];j++){
			for(int k=0;k<vektor_strojeva[N];k++){
				if(i==j){
					heuristicke_vrijednosti[i][j][k]=Beskonacnost;
				}
			}
		}
	}

	//određivanje početnog položaja mrava
	//mravi se raspoređuju kružno, po modulu aktivnosti
	vector<int> pom3;
	int mche=-1;
	int mch;
	for(int i=0;i<broj_mrava;i++){
		int jb=i%vektor_poslova[N];
		if (jb==0){
			mche++;
			mch=mche%vektor_strojeva[N];
		}
		pom3.push_back(jb);
		pom3.push_back(mch);
		pocetna_aktivnost_mrava.push_back(pom3);
		pom3.clear();
	}
	
	//upisivanje heurističkih vrijednosti u tablicu heurističkih vrijednsoti
	for(int i=0;i<vektor_poslova[N];i++){
		for(int j=0;j<vektor_poslova[N];j++){
			for(int k=0;k<vektor_strojeva[N];k++){
				if(i!=j)
					pom1.push_back(1.0/heuristicke_vrijednosti[i][j][k]);
				else
					pom1.push_back(1.0/Beskonacnost);
			}
			pom2.push_back(pom1);
			pom1.clear();
		}
		tablica_heuristickih_vrijednosti.push_back(pom2);
		pom2.clear();
	}
}

/*Funkcija inicijalizira tablicu feromonskih tragova.
Korisnik određuje inicijalnu vrijednost tau.
Za istu aktivnost se upisuje 0, tj. ne može mrav prelaziti u aktivnost u kojoj se nalazi*/
void inicijalizacija_feromonskog_traga(int N){

	vector<double> pom1;
	vector<vector<double> > pom2;

	//inicijalno resetiranje vrijednosti
	jacina_feromonskog_traga.clear();


	for(int i=0;i<vektor_poslova[N];i++){
		for(int j=0;j<vektor_poslova[N];j++){
			for(int k=0;k<vektor_strojeva[N];k++){
				if(i!=j)
					pom1.push_back(tau);
				else
					pom1.push_back(0);
			}
			pom2.push_back(pom1);
			pom1.clear();
		}
		jacina_feromonskog_traga.push_back(pom2);
		pom2.clear();
	}
}

/*Funkcija inicijalizira tabu listu svakog mrava.
Inicijalno se svakom mravu upisuje da je trajanje projekta koje je pronašao 0.
Inicijalno se aktivnost u kojoj se mrav nalazi postavlja u tabu listi u 0 (kao da je vec posjecena)*/
void inicijalizacija_mrava(int N){

	vector<int> pom1;
	
	//resetiram tabu listu jer se ucestalo koristi
	//inicijalno u listu svim mravima upisujem 1
	tabu_lista.clear();
	for(int i=0;i<broj_mrava;i++){
		for(int j=0;j<vektor_poslova[N];j++){
			pom1.push_back(1);
		}
		tabu_lista.push_back(pom1);
		pom1.clear();
	}

	//svim mravima inicijalno upisujem u trajanje projekta 0 -> to je Cmax ili neki drugi parametar koji vraća fitness fja

	trajanje_projekta.clear();
    for(int i=0;i<broj_mrava;i++)
	   trajanje_projekta.push_back(0);

	trenutna_aktivnost_mrava.clear();

	//upisujem u tabu listu na mjesto aktivnosti u kojoj se mrav nalazi 0
	//u trenutnu aktivnost mrava inicijalno stavljam početnu vrijednost
	for(int k=0;k<broj_mrava;k++){
		//mrav,posao
		tabu_lista[k][pocetna_aktivnost_mrava[k][0]]=0;
		trenutna_aktivnost_mrava.push_back(pocetna_aktivnost_mrava[k]);
	}
}

/*Funkcija za izračunavanje odlaska u sljedeću aktivnost na temelju stečenog znanja mrava.
Mrav je naučio da odlazi u onu aktivnost kojoj je umnožak feromonskog traga do te aktivnosti i njezine heurističke vrijednsoti najveća.
Funkcija se primjenjuje ukoliko je q<=q0	*/
vector<int> eksploatacija(int k, int N){

	double temp=0.0;
	vector<int> sljedeca_aktivnostp;
	for(int i=0;i<2;i++)
		sljedeca_aktivnostp.push_back(0);
	//trenutni posao mrava
	int lk=trenutna_aktivnost_mrava[k][0];
	//trenutni stroj
	int mch=trenutna_aktivnost_mrava[k][1];
	for(int i=0;i<vektor_poslova[N];i++){
		for(int j=0;j<vektor_strojeva[N];j++){
		//if(tabu_lista[k][i]==1 && lk!=i && heuristicke_vrijednosti[lk][i]!=Beskonacnost && temp < pow(jacina_feromonskog_traga[lk][i],alfa)*pow(tablica_heuristickih_vrijednosti[lk][i],beta))
			//ako mrav nije posjetio taj posao i ako se ponovno ne vraća u isti posao tada ...
			if(tabu_lista[k][i]==1 && lk!=i && temp < pow(jacina_feromonskog_traga[lk][i][j],alfa)*pow(tablica_heuristickih_vrijednosti[lk][i][j],beta))
			{
				temp=pow(jacina_feromonskog_traga[lk][i][j],alfa)*pow(tablica_heuristickih_vrijednosti[lk][i][j],beta);
				sljedeca_aktivnostp[0]=i;
				sljedeca_aktivnostp[1]=j;
			}
		}
	}
	return sljedeca_aktivnostp;
}

/*Funkcija za generiranje pseudoslučajnog broja u intervalu <0,1].
Funkcija se koristi prilikom odlučivanja hoće li mrav koristiti vjerojatnosnu funkciju ili stečeno znanje za odlazak u sljedeću aktivnost.*/
double drand()
{	
	return ((double)rand())/(double)Beskonacnost;
}

/*Funkcija za izračunavanje vjerojatnosti odlaska u sljedeću aktivnost.
Da bi mrav odlučio kamo odlazi, mora pogledati produkte feromonskog traga do aktivnosti i heursitičke vrijednosti i dodatno mora
zbrojiti sve te produkte da bi našao najveću vjerojatnost. Izračunavanje ove funkcije je dulje od eksploatacije jer mrav nije naučio
na temelju čega da vrši odabir odlaska u drugu aktivnost*/
vector<int> istrazivanje(int k, int N){

	int i;
	double sum=0;
	vector<int> pom;
	for(int i=0;i<2;i++)
		pom.push_back(0);
	//trenutni položaj mrava k
	int lk=trenutna_aktivnost_mrava[k][0]; 	
	//trenutni stroj
	int mch=trenutna_aktivnost_mrava[k][1];
	int count=0;
	double r=drand();
	
	//izračun ukupnog
	for(int i=0;i<vektor_poslova[N];i++){
		for(int j=0;j<vektor_strojeva[N];j++){
	
			//if(tabu_lista[k][i]==1&&heuristicke_vrijednosti[lk][i]!=Beskonacnost)
			//ako mrav ima dostupan odlazak u sljedece aktivnosti
			//zbrajam umnožak (nazivnik)
			if(tabu_lista[k][i]==1 && lk!=i){
				sum=sum+pow(jacina_feromonskog_traga[lk][i][j],alfa)*pow(tablica_heuristickih_vrijednosti[lk][i][j],beta);
				count++;
			}
		}
	}	
	double x=0;
	double y=0;
	for(int i=0;i<vektor_poslova[N];i++){
		for(int j=0;j<vektor_strojeva[N];j++){

			//if(tabu_lista[k][i]==1&&heuristicke_vrijednosti[lk][i]!=Beskonacnost)
			//radim vjerojatnosnu funkciju<karakteristicna funkcija za ACO>
			if(tabu_lista[k][i]==1 && lk!=i){	

				y=y+pow(jacina_feromonskog_traga[lk][i][j],alfa)*pow(tablica_heuristickih_vrijednosti[lk][i][j],beta);	
				//ako je slucajna vrijednst veca od x/(zbroj vjerojatnosti odlazaka) i manja od y/(-||-)
				if(r>=x/sum && r<y/sum){
					pom[0]=i;
					pom[1]=j;
					break;
				}
				else
					x=y;
			}
		}
	}
	//vrati index sljedece aktivnosti
	return pom;
}


/********************************************METODE ZA ACO********************************************/

/********************************************FITNESS********************************************/

/*Funkcija vraća 1 ukoliko su svi prethodnici aktivnosti u listi gotovih i ukoliko je posao spreman za obradu, u suprotnom vraća 0*/
int podskup_gotovih_i_analiziranih(int prethode, vector<int> lista_gotovih,int trenutno_vrijeme, int pripravnost){
	int j, br = 1;
	//ukoliko posao još nije pripravan, prekini
	if(trenutno_vrijeme<pripravnost){
		return 0;
	}

	for (j=0; j<lista_gotovih.size(); j++) {
		if (prethode == lista_gotovih[j]) {
			br -= 1;
			break;
		}
	}
	
	if (br == 0) return 1;
	return 0;
}

/*Metoda za računanje najmanje razlike pripravnosti */
int razlika_vremena(vector<posao> neprocesuirani,int trenutno_vrijeme,vector<int> gotove,int max_prioritet){

	vector<posao> zavrseni;
	int min_pripravnost;
	int delta=0;

	//potraži sve koji čiji su prethodnici završili U NEPROCESUIRANIM!
	for(int i=0;i<neprocesuirani.size();i++){
		for(int j=0;j<gotove.size();j++){
			if(neprocesuirani[i].prethodnik==gotove[j]){
				zavrseni.push_back(neprocesuirani[i]);
				break;
			}
		}
		//ne treba se tražiti poslove čiji prioritet je veći max_prioritet+1 
		if(neprocesuirani[i].prioritet>(max_prioritet+1))
			break;
	}

	//potraži najmanju pripravnost kod poslova čiji prethodnici su završili jer taj posao započinje prvi od nezapočetih
	if(zavrseni.size()!=0){
		min_pripravnost=zavrseni[0].Rpripravnost;
	}else
		return 0;

	for(int i=1;i<zavrseni.size();i++){
		if(zavrseni[i].Rpripravnost<min_pripravnost)
			min_pripravnost=zavrseni[i].Rpripravnost;
	}

	//delta je vrijeme potrebno da posao čiji prethodnik je završio da postane pripravan i spreman za pokretanje
	delta=min_pripravnost-trenutno_vrijeme;

	return delta;
}

/*Metoda racuna prethodni posao posla j na stroju i*/
void izracunaj_prethodnika(int N){

	vector<posao> jobmac;
	posao zamjena;
	for(int i=0;i<vektor_strojeva[N];i++){
		for(int j=0;j<poslovi.size();j++){
			if(poslovi[j].rb_stroj==(i+1)){
				jobmac.push_back(poslovi[j]);
			}
		}
		//soritraj taj vektor po prioritetima
		for(int k=0;k<jobmac.size();k++){
			for(int m=k+1;m<jobmac.size();m++){
				if(jobmac[k].prioritet>jobmac[m].prioritet){
					zamjena=jobmac[k];
					jobmac[k]=jobmac[m];
					jobmac[m]=zamjena;
				}
			}
		}
		for(int n=0;n<jobmac.size();n++){
			for(int p=0;p<poslovi.size();p++){
				if(jobmac[n].brposla==poslovi[p].brposla){
					if(n==0){
						poslovi[p].prethodnik=0;
					}else{
						poslovi[p].prethodnik=jobmac[n-1].brposla;
					}
				}
			}
		}
		jobmac.clear();
	}
}

/*Metoda koja provjeri je li posao j dio redoslijeda obilaska*/
int in_redoslijed_obilaska(int j,vector<vector<int> > redoslijed_obilaska){
	for(int i=0;i<redoslijed_obilaska.size();i++){
		if((j+1)==redoslijed_obilaska[i][0]){
			return 1;
		}
	}

	return 0;
}

void izracunaj_prethodnika_novi(int N,vector<vector<int> > redoslijed_obilaska){

	vector<posao> jobmac;
	posao zamjena;
	for(int i=0;i<vektor_strojeva[N];i++){
		for(int j=0;j<poslovi.size();j++){
			if((poslovi[j].rb_stroj==(i+1)) && in_redoslijed_obilaska(j,redoslijed_obilaska)){
				jobmac.push_back(poslovi[j]);
			}
		}
		//soritraj taj vektor po prioritetima
		for(int k=0;k<jobmac.size();k++){
			for(int m=k+1;m<jobmac.size();m++){
				if(jobmac[k].prioritet>jobmac[m].prioritet){
					zamjena=jobmac[k];
					jobmac[k]=jobmac[m];
					jobmac[m]=zamjena;
				}
			}
		}
		for(int n=0;n<jobmac.size();n++){
			for(int p=0;p<poslovi.size();p++){
				if(jobmac[n].brposla==poslovi[p].brposla){
					if(n==0){
						poslovi[p].prethodnik=0;
						//ovo je da bih kasnije mogao spojiti sve poslove u jednu povezanu listu s prethodnicima
						poslovi[p].prethodnik_povezi=job_prethodi[i];
					}else{
						poslovi[p].prethodnik=jobmac[n-1].brposla;
					}
				}
			}
		}
		int jobmac_index=jobmac.size();
		if(jobmac_index!=0)
			job_prethodi[i]=jobmac[jobmac.size()-1].brposla;
		jobmac.clear();
	}
}

/*Metoda za simulaciju obrade poslova na strojevima
AKO JE 1, TADA GA KORISTI ACO, AKO JE 0, KORISTI GA MIN_MIN,  A AKO JE -1, TADA GA KORISTI ACO S RASPOREĐIVANJEM NA ZAHTJEV  */
int fitness(vector<vector<int> > redoslijed_obilaska,int parametar,int N){    
	

	posao zamjena;	//varijabla za zamjenu u selection sortu
	posao temp;		//varijabla za pokretanje obrade poslova
	posao trenutni;	//varijabla za brisanje iz pokrenutih
	int duljina=0;
	int br=0;
	int trenutno_vrijeme=0;	//trenutno vrijeme za mjerenje ostalih parametara
	double akumulator=0;	//varijabla za spremanje povratne vrijednosti
	int delta_time=0;		//razlika dvaju najmanjih vremena pripravnosti
	int max_pripravnost=0;	//maksimalna pripravnost
	int max_prioritet=0;	//maksimalni prioritet u listi pokrenutih

	//vektor gotovih poslova
	vector<int> gotove;
	//vektor pokrenutih poslova
	vector<posao> pokrenute;
	//vektor neprocesuiranih poslova
	vector<posao> neprocesuirani;

	if(parametar==1){
		//OVO JE ZA ACO U FITNESS

		//1. treba rasporediti strojeve i prioritete u poslove
		//2. pozvati racunaj prethodnika

		//1.
		vector<int> prior_na_str;
		for(int i=0;i<vektor_strojeva[N];i++)
			prior_na_str.push_back(0);
		for(int i=0;i<redoslijed_obilaska.size();i++){
			for(int j=0;j<poslovi.size();j++){
				if(redoslijed_obilaska[i][0]==poslovi[j].brposla){
					poslovi[j].rb_stroj=redoslijed_obilaska[i][1];
					poslovi[j].prioritet=prior_na_str[poslovi[j].rb_stroj-1];
					prior_na_str[poslovi[j].rb_stroj-1]++;
					break;
				}
			}
		}
		//2.
		izracunaj_prethodnika(N);

		for(int i=0;i<poslovi.size();i++)
			neprocesuirani.push_back(poslovi[i]);

	}else if(parametar==0){
		//OVO JE ZA MIN-MIN 
		for(int i=0;i<poslovi.size();i++)
			neprocesuirani.push_back(poslovi[i]);
	}else if(parametar==-1){

		//vector<int> prioritet_na_stroju_zahtjev, on je postavljen na 0 kod ACO na zahtjev

		for(int i=0;i<redoslijed_obilaska.size();i++){
			for(int j=0;j<poslovi.size();j++){
				if(redoslijed_obilaska[i][0]==poslovi[j].brposla){
					poslovi[j].rb_stroj=redoslijed_obilaska[i][1];
					poslovi[j].prioritet=prioritet_na_stroju_zahtjev[poslovi[j].rb_stroj-1];
					prioritet_na_stroju_zahtjev[poslovi[j].rb_stroj-1]++;
					break;
				}
			}
		}

		izracunaj_prethodnika_novi(N,redoslijed_obilaska);

		for(int i=0;i<poslovi.size();i++){
			if(in_redoslijed_obilaska(i,redoslijed_obilaska)){
				neprocesuirani.push_back(poslovi[i]);
			}
		}

	}else if(parametar==2){

		for(int i=0;i<poslovi.size();i++){
			if((poslovi[i].prethodnik_povezi>0)&&(poslovi[i].prethodnik_povezi<(poslovi.size())))
				poslovi[i].prethodnik=poslovi[i].prethodnik_povezi;
			neprocesuirani.push_back(poslovi[i]);
		}

		for(int i=0;i<neprocesuirani.size();i++)
			cout<<neprocesuirani[i].brposla<<" "<<neprocesuirani[i].prioritet;cout<<endl;
	}

	//uzalzno sortiranje poslova po prioritetu
	//selection sort
	for(int i=0;i<neprocesuirani.size();i++){
		for(int j=i+1;j<neprocesuirani.size();j++){
			if(neprocesuirani[i].prioritet>neprocesuirani[j].prioritet){
				zamjena=neprocesuirani[i];
				neprocesuirani[i]=neprocesuirani[j];
				neprocesuirani[j]=zamjena;
			}	
		}
	}

	//traženje najveće pripravnosti
	for(int i=0;i<neprocesuirani.size();i++){
		if(neprocesuirani[i].Rpripravnost>max_pripravnost)
				max_pripravnost=neprocesuirani[i].Rpripravnost;
	}

	//0. posao je fiktivan i on je apriori završio
	gotove.push_back(0);

	//obrada trajanja
	//dok god postoje neprocesuirani poslovi
	while(!neprocesuirani.empty()){
		
		duljina=neprocesuirani.size();
		br=0;

		//za svaki posao
		for(int i=0; i<duljina;i++){
			//trenutni posao
			temp=neprocesuirani[i-br];

			//provjeri jesu li završili prethodnici
			if(podskup_gotovih_i_analiziranih(temp.prethodnik,gotove,trenutno_vrijeme,temp.Rpripravnost)){
				//zabilježi koji je max prioritet
				//to bilježim zbog kasnije provjere koji poslovi bi možda mogli biti pripravni za rad
				if(temp.prioritet>max_prioritet)
					max_prioritet=temp.prioritet;
				//ukoliko su prethodni poslovi zavrsili, pokreni posao
				pokrenute.push_back(temp);
				//izbrisi trenutni posao iz neprocesuiranih
				//neprocesuirani.erase(neprocesuirani.begin()+i-br);
				for (int ii=i-br; ii<neprocesuirani.size()-1; ii++){
					neprocesuirani[ii].brposla=neprocesuirani[ii+1].brposla;
					neprocesuirani[ii].Deadline=neprocesuirani[ii+1].Deadline;
					neprocesuirani[ii].Rpripravnost=neprocesuirani[ii+1].Rpripravnost;
					neprocesuirani[ii].Wtezina=neprocesuirani[ii+1].Wtezina;
					neprocesuirani[ii].rb_stroj=neprocesuirani[ii+1].rb_stroj;
					neprocesuirani[ii].prioritet=neprocesuirani[ii+1].prioritet;
					neprocesuirani[ii].prethodnik=neprocesuirani[ii+1].prethodnik;
					neprocesuirani[ii].trajanje.swap(neprocesuirani[ii+1].trajanje);
					neprocesuirani[ii].prethodnik_povezi=neprocesuirani[ii+1].prethodnik_povezi;
				}
				neprocesuirani.pop_back();
				br+=1;
			}
		}

			//ukoliko postoje pokrenuti poslovi
label1:		if(!pokrenute.empty()){

			duljina=pokrenute.size();
			temp=pokrenute[0];
			int min=temp.trajanje[temp.rb_stroj-1];
			int minadr=0;
			int minprip=0;
			int koji_posao=temp.brposla;

			

			//koliko traje i koji je posao od trenutnih najkraći
			for(int i=1;i<duljina;i++){
				temp=pokrenute[i];
				if(temp.trajanje[temp.rb_stroj-1]<min){
					min=temp.trajanje[temp.rb_stroj-1];
					minadr=i;
					koji_posao=temp.brposla;

				}
			}

			if(trenutno_vrijeme<max_pripravnost){
				//racuna razliku vremena između prvog posla čiji prethodnik je završio, a čeka da postane pripravan i pokrenutog posla čije trajanje je minimalno
				delta_time=razlika_vremena(neprocesuirani,trenutno_vrijeme,gotove,max_prioritet);
			}else{
				delta_time=0;
			}

			if((delta_time>=min) || (delta_time==0)){

				//izbriši taj posao iz liste pokrenutih i zapamti ga jer nad gotovim poslom se treba provesti statistika
				trenutni=pokrenute[minadr];
				pokrenute.erase(pokrenute.begin()+minadr);

				duljina=pokrenute.size();

				//oduzmi proteklo vrijeme
				for(int i=0; i<duljina; i++){
					pokrenute[i].trajanje[pokrenute[i].rb_stroj-1]-=trenutni.trajanje[trenutni.rb_stroj-1]; //oduzimanje vremena procesiranja na stroju obrade
				}
			
				//u listu gotovih dodaj posao koja je završio
				gotove.push_back(trenutni.brposla);

				trenutno_vrijeme+=trenutni.trajanje[trenutni.rb_stroj-1];

				//upisivanje podataka u posao u listi poslova
				//indexi poslova u listi počinju od 1!
				poslovi[trenutni.brposla-1].Czavrsetak=trenutno_vrijeme;
				poslovi[trenutni.brposla-1].kasnjenje();
				poslovi[trenutni.brposla-1].protjecanje();
				poslovi[trenutni.brposla-1].zaostajanje();
				poslovi[trenutni.brposla-1].zakasnjelost();
				
			}else{
				for(int i=0;i<pokrenute.size();i++){
					pokrenute[i].trajanje[pokrenute[i].rb_stroj-1]-=delta_time;
				}
					trenutno_vrijeme+=delta_time;
			}

			//uvjet koji prazni listu pokrenutih ukoliko više ne postoje neprocesuirane aktivnosti
			if(neprocesuirani.empty()){
				if(!pokrenute.empty()){
					goto label1;
				}
				else
					break;
			}

		}else{
			//ukoliko je red pokrenutih prazan, povećaj vrijeme
			trenutno_vrijeme++;
			}

	}

	//fitness može vraćati 4 moguće vrijednosti: Cmax / Fw / Tw / Uw
	//u odnosu na ono što je odabrao korisnik, to će fitness vraćati

	double F=0;
	double U=0;
	double T=0;
	if(parametar!=-1){
	for(int i=0; i<poslovi.size();i++){
		if(poslovi[i].Czavrsetak>akumulator)
			akumulator=poslovi[i].Czavrsetak;
		F+=poslovi[i].Wtezina*poslovi[i].Fprotjecanje;
		T+=poslovi[i].Wtezina*poslovi[i].Tzaostajanje;
		U+=poslovi[i].Wtezina*poslovi[i].Uzakasnjelost;
	}
	dobrota_rasporeda.Cnaj=akumulator;
	dobrota_rasporeda.Fnaj=F;
	dobrota_rasporeda.Tnaj=T;
	dobrota_rasporeda.Unaj=U;
	}else{

		for(int i=0;i<redoslijed_obilaska.size();i++){
			for(int j=0;j<poslovi.size();j++){
				if((redoslijed_obilaska[i][0])==poslovi[j].brposla){
					if(poslovi[j].Czavrsetak>akumulator){
						akumulator=poslovi[j].Czavrsetak;
					}
					F+=poslovi[j].Wtezina*poslovi[j].Fprotjecanje;
					T+=poslovi[j].Wtezina*poslovi[j].Tzaostajanje;
					U+=poslovi[j].Wtezina*poslovi[j].Uzakasnjelost;
				}
			}
		}
	}

	if(kriterij_optimizacije==1){
		return akumulator;
	}else if(kriterij_optimizacije==2){
		return F;
	}else if(kriterij_optimizacije==3){
		return T;
	}else if(kriterij_optimizacije==4){
		return U;
	}
}

/********************************************FITNESS********************************************/

/********************************************CITANJA DATOTEKA********************************************/

void citanje_glavne_datoteke(){

	//varijabla za učitavanje redova
	string redak;
	//konstanta komentar
	string komentar="//";
	//ime glavne ulazne datoteke
	ifstream ulazna("fitness.txt");

	if(!ulazna){
		cout<<"Pogreska prilikom otvaranja datoteke!"<<endl;
		return;
	}else{
		cout<<"Datoteka fitness.txt uspjesno otvorena."<<endl;
	}

	//citanje datoteke red po red
	while(getline(ulazna,redak)){

		string poms;
		stringstream sTok(redak);
		//ucitavam sTok u poms
		sTok>>poms;
		int pomi;

		if(poms==komentar){
			continue;
		}

		if(poms=="sets"){
			sTok>>poms;
			istringstream sToks(poms);
			sToks>>konstante.broj_skupova;
			continue;
		}

		if(poms=="max_jobs"){
			sTok>>poms;
			istringstream sToks(poms);
			sToks>>konstante.max_br_poslova;
			continue;
		}

		if(poms=="max_machines"){
			sTok>>poms;
			istringstream sToks(poms);
			sToks>>konstante.max_br_strojeva;
			continue;
		}

		if(poms=="max_length"){
			sTok>>poms;
			istringstream sToks(poms);
			sToks>>konstante.max_trajanje_posla;
			continue;
		}

		if(poms=="jobs"){
			while(sTok>>pomi){
				vektor_poslova.push_back(pomi);
			}
			continue;
		}

		if(poms=="machines"){
			while(sTok>>pomi){
				vektor_strojeva.push_back(pomi);
			}
			continue;
		}

		if(poms=="ready"){
			sTok>>poms;
			dolasci_poslova=poms;
			continue;
		}

		if(poms=="duration"){
			sTok>>poms;
			trajanja_poslova=poms;
			continue;
		}

		if(poms=="deadline"){
			sTok>>poms;
			zavrsetci_poslova=poms;
			continue;
		}

		if(poms=="weight_F"){
			sTok>>poms;
			tezinski_faktori_poslova=poms;
			break;
		}
	}

	cout<<"Procitana datoteka fitness.txt."<<endl;
	cout<<"Ucitani podaci o poslovima:"<<endl;
	cout<<"		1.) Broj skupova: "<<konstante.broj_skupova<<endl;
	cout<<"		2.) Najveci broj poslova u skupovima: "<<konstante.max_br_poslova<<endl;
	cout<<"		3.) Najveci broj strojeva u skupovima: "<<konstante.max_br_strojeva<<endl;
	cout<<"		4.) Najdulje trajanje: "<<konstante.max_trajanje_posla<<endl;
	cout<<endl;
	cout<<"Podaci o poslova po skupovima:"<<endl;
	cout<<"		1.) Broj poslova je: "<<vektor_poslova.size()<<endl;
	cout<<endl;
	cout<<endl;
	cout<<"Podaci o strojevima u skupovima:"<<endl;
	cout<<"		1.) Broj strojeva je: "<<vektor_strojeva.size()<<endl;
	cout<<endl;
	cout<<endl;
	cout<<"Podaci o ulaznim datotekama:"<<endl;
	cout<<"		1.) Datoteka s vremenima dolazaka poslova:"<<dolasci_poslova<<endl;
	cout<<"		2.) Datoteka s vremenima trajanja poslova:"<<trajanja_poslova<<endl;
	cout<<"		3.) Datoteka s vremenima zavrsetaka poslova:"<<zavrsetci_poslova<<endl;
	cout<<"		4.) Datoteka s tezinskim faktorima poslova:"<<tezinski_faktori_poslova<<endl;
}

/*Metoda za citanje pripravnosti poslova ili dolazaka poslova u sustav.
Ne čita se cijela datoteka odjednom, nego se datoteka čite red po red kako se informacije obrađuju
citam N+1 red, pocevsi brojati od 0. */
void citanje_dolazaka_poslova(int N){

	//varijabla za učitavanje redova
	string redak;
	//brojac redaka
	int brojac=0;
	//pocetna vrijednost u datoteci
	int redovi, kolone;
	//broj poslova u ovom ispitivanju
	int broj_aktivnih_poslova=vektor_poslova[N];

	//ime glavne ulazne datoteke
	ifstream ulazna(dolasci_poslova.c_str());

	if(!ulazna){
		cout<<"Pogreska prilikom otvaranja datoteke! Datoteka "<<dolasci_poslova<<" nije uspjesno otvorena ili se ne nalazi u trenutnom direktoriju"<<endl;
		return;
	}

	//citanje datoteke red po red
	while(getline(ulazna,redak)){

		string poms;
		stringstream sTok(redak);
		//ucitavam sTok u poms
		int pomi;

		if(brojac==0){
			sTok>>redovi;
			sTok>>kolone;

			if((redovi!=konstante.broj_skupova) || (kolone!=konstante.max_br_poslova)){
				cout<<"Procitana datoteka nije kompatibilna sa zadanim problemom. Prekid rada."<<endl;
				return;
			}
		}

		if(brojac==(N+1)){
			int i=0;
			while(sTok>>pomi){
				poslovi[i].Rpripravnost=pomi;
				i++;

				if(i==broj_aktivnih_poslova)
					break;
			}
			break;
		}

		brojac++;
	}
}


/*Metoda za citanje trajanja poslova na određenosm stroju
Za svaki posao je zapisano trajanje obrade na svakom poslu
citam N+1 red, pocevsi brojati od 0. */
void citanje_trajanja_poslova(int N){

	//varijabla za učitavanje redova
	string redak;
	//brojac redaka
	int brojac=0;
	//pocetna vrijednost u datoteci
	int redovi, kolone;
	//broj poslova u ovom ispitivanju
	int broj_aktivnih_poslova=vektor_poslova[N];
	//broj strojeva u ovom ispitivanju
	int broj_aktivnih_strojeva=vektor_strojeva[N];
	//broj podataka za jedno ispitivanje
	int broj_podataka=broj_aktivnih_poslova*broj_aktivnih_strojeva;

	//ime glavne ulazne datoteke
	ifstream ulazna(trajanja_poslova.c_str());

	if(!ulazna){
		cout<<"Pogreska prilikom otvaranja datoteke! Datoteka "<<dolasci_poslova<<" nije uspjesno otvorena ili se ne nalazi u trenutnom direktoriju"<<endl;
		return;
	}

	//citanje datoteke red po red
	while(getline(ulazna,redak)){

		string poms;
		stringstream sTok(redak);
		//ucitavam sTok u poms
		int pomi;

		if(brojac==0){
			sTok>>redovi;
			sTok>>kolone;

			if((redovi!=konstante.broj_skupova) || (kolone!=(konstante.max_br_poslova*konstante.max_br_strojeva))){
				cout<<"Procitana datoteka nije kompatibilna sa zadanim problemom. Prekid rada."<<endl;
				return;
			}
		}

		if(brojac==(N+1)){

			for(int i=0;i<broj_aktivnih_poslova;i++){
				for(int j=0;j<broj_aktivnih_strojeva;j++){
					sTok>>pomi;
					poslovi[i].trajanje.push_back(pomi);
				}
			}
			break;
		}

		brojac++;
	}
}


/*Metoda za citanje vremena željenog završetka
citam N+1 red, pocevsi brojati od 0. */
void citanje_zavrsetaka_poslova(int N){

	//varijabla za učitavanje redova
	string redak;
	//brojac redaka
	int brojac=0;
	//pocetna vrijednost u datoteci
	int redovi, kolone;
	//broj poslova u ovom ispitivanju
	int broj_aktivnih_poslova=vektor_poslova[N];

	//ime glavne ulazne datoteke
	ifstream ulazna(zavrsetci_poslova.c_str());

	if(!ulazna){
		cout<<"Pogreska prilikom otvaranja datoteke! Datoteka "<<dolasci_poslova<<" nije uspjesno otvorena ili se ne nalazi u trenutnom direktoriju"<<endl;
		return;
	}

	//citanje datoteke red po red
	while(getline(ulazna,redak)){

		string poms;
		stringstream sTok(redak);
		//ucitavam sTok u poms
		int pomi;

		if(brojac==0){
			sTok>>redovi;
			sTok>>kolone;

			if((redovi!=konstante.broj_skupova) || (kolone!=konstante.max_br_poslova)){
				cout<<"Procitana datoteka nije kompatibilna sa zadanim problemom. Prekid rada."<<endl;
				return;
			}
		}

		if(brojac==(N+1)){
			int i=0;
			while(sTok>>pomi){
				poslovi[i].Deadline=pomi;
				i++;

				if(i==broj_aktivnih_poslova)
					break;
			}
			break;
		}

		brojac++;
	}
}

/*Metoda za čitanje težinskih faktora poslova
citam N+1 red, pocevsi brojati od 0. */
void citanje_tezinskih_faktora_poslova(int N){

	//varijabla za učitavanje redova
	string redak;
	//brojac redaka
	int brojac=0;
	//pocetna vrijednost u datoteci
	int redovi, kolone;
	//broj poslova u ovom ispitivanju
	int broj_aktivnih_poslova=vektor_poslova[N];
	string pomi;

	//ime glavne ulazne datoteke
	ifstream ulazna(tezinski_faktori_poslova.c_str());

	if(!ulazna){
		cout<<"Pogreska prilikom otvaranja datoteke! Datoteka "<<dolasci_poslova<<" nije uspjesno otvorena ili se ne nalazi u trenutnom direktoriju"<<endl;
		return;
	}

	//citanje datoteke red po red
	while(getline(ulazna,redak)){

		string poms;
		stringstream sTok(redak);
		//ucitavam sTok u poms

		if(brojac==0){
			sTok>>redovi;
			sTok>>kolone;

			if((redovi!=konstante.broj_skupova) || (kolone!=konstante.max_br_poslova)){
				cout<<"Procitana datoteka nije kompatibilna sa zadanim problemom. Prekid rada."<<endl;
				return;
			}
		}

		if(brojac==(N+1)){
			int i=0;
			while(sTok>>pomi){
				istringstream stm;
				stm.str(pomi);
				stm>>poslovi[i].Wtezina;
				//i->i+1
				poslovi[i].brposla=i+1;
				i++;

				if(i==broj_aktivnih_poslova)
					break;
			}
			break;
		}

		brojac++;
	}
}

/********************************************CITANJA DATOTEKA********************************************/

/********************************************ALGORITAMSKE METODE ZA MIN-MIN I ACO********************************************/

/*Metoda za inicijalizaciju globalnog vrektora poslova */
void inicijaliziraj_vektor_poslova(int N){
	int br_job=vektor_poslova[N];
	posao prazna_struktura_podataka;

	for(int i=0;i<br_job;i++){
		poslovi.push_back(prazna_struktura_podataka);
	}
}

/*Metoda za računanje najmanjeg vremena pripravnosti od svih strojeva */
int racunaj_ro(vector<int> pripr_strojeva){

	int min=32000;
	for(int i=0;i<pripr_strojeva.size();i++){
		if((min>pripr_strojeva[i])&&(pripr_strojeva[i]!=0))
			min=pripr_strojeva[i];
	}

	if(min==32000){
		return 0;
	}else{
		return min;
	}
}

/*delta=R(dolazeci[min])-tren_time */
int racunaj_delta(vector<posao> dolazeci, int tren_time){

	if(dolazeci.size()!=0){
		int min=dolazeci[0].Rpripravnost;
		return (min-tren_time);
	}else{
		return 0;
	}

}

/********************************************ALGORITAMSKE METODE ZA MIN-MIN I ACO********************************************/

/********************************************MIN-MIN********************************************/
/*MIN_MIN algoritam za PRIDRUŽIVANJE I UREĐIVANJE poslova
Algoritam je opisan na 78. str. doktorske disertacije doc.dr.sc. Jakobovića*/
void min_min(int N){

	int min_c=0;
	int indeks_stroj;
	int indeks_posao;
	int ro,delta;
	posao zamjena;

	clock_t pocetaK, kraJ;
	double sumator=0;
	int brojraspored=0;

	//skup svih raspoloživih poslova koji još nisu pokrenuti, inicijalno je Jmeta prazan
	vector<posao> Jmeta;
	//vektor za prećenje dodjele prioriteta na nekom stroju
	vector<int> prati_prioritete;
	//pripravnost strojeva, inicijalno je 0
	vector<int> pripr_strojeva;
	//vektor neprocesuiranih poslova
	vector<posao> neprocesuirani;
	//vektor bool koji stroj trenutno radi
	vector<int> zaposlen;
	//poslovi koji su dolazeci
	vector<posao> dolazeci;
	//vektor velicine JMETA.size() za ispis
	vector<int> Jsize;

	izlazRAS<<"Trajanje_rasporedivanja_za_"<<N<<"._skup._Vrijednosti_su_izrazene_u_milisekundama."<<endl;
	izlazRAS<<"MIN-MIN: ";

	//kopija poslova u dolazece
	for(int i=0;i<poslovi.size();i++){
		dolazeci.push_back(poslovi[i]);
	}

	//početna inicijalizacija prioriteta na strojevima, pripreme stroja
	for(int i=0;i<vektor_strojeva[N];i++){
		prati_prioritete.push_back(0);
		pripr_strojeva.push_back(0);
		zaposlen.push_back(0);
	}

	//sortiranje dolazecih poslova po pripravnosti, uzlazno
	for(int i=0;i<dolazeci.size();i++){
		for(int j=i+1;j<dolazeci.size();j++){
			if(dolazeci[i].Rpripravnost>dolazeci[j].Rpripravnost){
				zamjena=dolazeci[i];
				dolazeci[i]=dolazeci[j];
				dolazeci[j]=zamjena;
			}
		}
	}

	//trenutno vrijeme postaje pripravno vrijeme
	int tren_time=dolazeci[0].Rpripravnost;
	int uzi=0;
	while(uzi<dolazeci.size()){

		if(tren_time==dolazeci[uzi].Rpripravnost){
			Jmeta.push_back(dolazeci[uzi]);
			dolazeci.erase(dolazeci.begin()+uzi);
			uzi=0;
			continue;
		}
		uzi++;
	}

	//VELIKA PETLJA
	//dok posotje dolazeći ili postoje neprocesuirani
	while((dolazeci.size()>0) || (neprocesuirani.size()>0)|| (Jmeta.size()>0) ){

		pocetaK=clock();
		
		neprocesuirani.clear();
		//matrica za Cij, tu se racunaju vremena zavrsetaka posla j na stroju i
		vector<vector<int> > C;

		Jsize.push_back(Jmeta.size());

		//pronaci sve Cij
		for(int i=0;i<Jmeta.size();i++){
			vector<int> trajanje_posla_ij;
			for(int j=0;j<pripr_strojeva.size();j++){
				//racunanje zavrsetka obrade posla j na stroju i, s obzirom na pripravnost stroja i trajanje obrade na tom stroju
				trajanje_posla_ij.push_back(pripr_strojeva[j]+Jmeta[i].trajanje[j]);	//Cij=tri+pij
			}
			//stavljanje u matricu
			C.push_back(trajanje_posla_ij);
		}

		vector<int> copy_ps;
		for(int i=0;i<pripr_strojeva.size();i++)
			copy_ps.push_back(pripr_strojeva[i]);

		//dok postoje poslovi u Jmeta radi...
		//ovdje se vrsi pridruzivanje i uređivanje -> koji posao se radi na kojem stroju i koji je po redu u redoslijedu obrade
		while(!Jmeta.empty()){
			
			//za svaki zadatak iz Jmeta pronaći najmanji Cij i pronaći koji je to posao i na kojem stroju
			min_c=C[0][0];
			//strojevi idu od 1. i poslovi idu od 1
			indeks_stroj=1;
			indeks_posao=0;
			//pronađi najmanji Cij u cijeloj matrici
			for(int i=0;i<Jmeta.size();i++){
				for(int j=0;j<vektor_strojeva[N];j++){
					if(C[i][j]<min_c){
						//pridruživanje posla stroju
						indeks_stroj=j;
						//indeks posla u Jmeta
						indeks_posao=i;
						min_c=C[i][j];
					}
				}

			}
			
			//dodjeljivanje posla stroju i uređivanje 
			Jmeta[indeks_posao].rb_stroj=indeks_stroj+1;	//strojevi počinju od indeksa 1
			//+1 jer prioriteti počinju od indexa 1
			Jmeta[indeks_posao].prioritet=prati_prioritete[indeks_stroj]+1;
			//povećam broj prioriteta sljedećeg posla
			prati_prioritete[indeks_stroj]++;
			//obnoviti vrijeme pripravnosti stroja na kojem sam pokrenuo posao
			pripr_strojeva[indeks_stroj]+=Jmeta[indeks_posao].trajanje[indeks_stroj];
			//stavljanje poslova u novi vektor radi čuvanja informacije
			neprocesuirani.push_back(Jmeta[indeks_posao]);
			//brisanje posla iz Jmeta
			Jmeta.erase(Jmeta.begin()+indeks_posao);
			//brisanje iz matrice Cij trajanja za posao indeks_posao
			C.erase(C.begin()+indeks_posao);

			//obnoviti Cij za prostale poslove na tom stroju
			for(int i=0;i<Jmeta.size();i++){
				C[i][indeks_stroj]=pripr_strojeva[indeks_stroj]+Jmeta[i].trajanje[indeks_stroj];
			}	
		}

		for(int i=0;i<copy_ps.size();i++)
			pripr_strojeva[i]=copy_ps[i];

		//postoji li koji slobodan stroj za obradu poslova i koji je to
		for(int i=0;i<vektor_strojeva[N];i++){
			int pokrecem_posao=32000;
			//ako je stroj slobodan
			if((zaposlen[i]==0)&&(neprocesuirani.size()!=0)){
				//pronađi posao čiji prioritet izvođenja na tom stroju je najmanji
				for(int j=0;j<neprocesuirani.size();j++){
					if((neprocesuirani[j].rb_stroj-1==i)&&(neprocesuirani[j].prioritet<=prati_prioritete[i])){
						prati_prioritete[i]=neprocesuirani[j].prioritet;
						pokrecem_posao=j;
					}
				}

				if(pokrecem_posao==32000)
					continue;

				//kad je posao pronađen, on se pokreće na tom stroju, odnosno, briše se posao iz neprocesuranih, a povećava se vrijeme priprave stroja i za trajanje posla j
				for(int k=0;k<poslovi.size();k++){
					if(poslovi[k].brposla==neprocesuirani[pokrecem_posao].brposla){
						//upisivanje podataka u posao
						poslovi[k].rb_stroj=neprocesuirani[pokrecem_posao].rb_stroj;	
						poslovi[k].prioritet=neprocesuirani[pokrecem_posao].prioritet;		
						//zaposli stroj
						zaposlen[i]=1;
						//povecaj vrijeme pripravnosti stroja na kojem se izvrsava obrada
						pripr_strojeva[i]=neprocesuirani[pokrecem_posao].trajanje[i];
						//posto je posao pokrenut, smanji broj neprocesuiranih poslova
						neprocesuirani.erase(neprocesuirani.begin()+pokrecem_posao);
						break;
					}
				}
			}
		}

		for(int i=0;i<neprocesuirani.size();i++)
			Jmeta.push_back(neprocesuirani[i]);


		//sad trebam imati 3 informacije: 1.trenutno vrijeme  2.najmanja priprava strojeva  3.najmanja priprava poslova
		//te 3 informacije kombinira funkcija: racunaj_vrijeme
		
		//vraca vrijednost najmanje obrade na svim strojevima
		ro=racunaj_ro(pripr_strojeva);
		//vraca vrijeme potrebno da bi sljedeći posao postao pripravan iz dolazećih
		delta=racunaj_delta(dolazeci,tren_time);

		//ako je vrijeme za pripravu sljedećeg posla manje ili jednako vremenu obrade posla na nekom od strojeva
		if((ro<=delta) || (delta==0)){
			for(int i=0;i<vektor_strojeva[N];i++){
				//oduzmi trajanje procesiranja na tom stroju
				if(pripr_strojeva[i]!=0){
					pripr_strojeva[i]-=ro;
					//ako je vrijednost pripravnosti stroja jednaka 0, onda je taj stroj odmah spreman za posao
					if(pripr_strojeva[i]==0){
						zaposlen[i]=0;
					}
				}
			}
		}else{
			//ako je vrijeme dolaska u sustav manje od obrade
			for(int i=0;i<vektor_strojeva[N];i++){
				if(pripr_strojeva[i]!=0)
					pripr_strojeva[i]-=delta;
			}
		}

		//uvećaj trenutno vrijeme
		if(delta!=0)
			tren_time+=delta;
		else
			tren_time+=ro;

		//pokreni sve poslove koji mogu početi
		int ii=0;
		while(ii<dolazeci.size()){

			if (tren_time<dolazeci[ii].Rpripravnost)
				break;

			if(tren_time==dolazeci[ii].Rpripravnost){
				Jmeta.push_back(dolazeci[ii]);
				dolazeci.erase(dolazeci.begin()+ii);
				ii=0;
				continue;
			}
			ii++;
		}

		kraJ=clock();
		izlazRAS<<(double(kraJ-pocetaK)/(CLOCKS_PER_SEC/1000))<<" ";
		sumator+=(double(kraJ-pocetaK)/(CLOCKS_PER_SEC/1000));
		brojraspored++;
	}
	izlazRAS<<endl;
	izlazRAS<<"JMETA[MIN-MIN]: ";
	for (int i=0;i<Jsize.size();i++){
		izlazRAS<<Jsize[i]<<" ";
	}
	cout<<"Prosjecno trajanje jednog rasporedivanja algoritma MIN-MIN je "<<(double)(sumator/brojraspored)<<" ms."<<endl;
	izracunaj_prethodnika(N);
}

/********************************************MIN-MIN********************************************/

/********************************************ACO********************************************/

/*ACO algoritam za PRIDRUŽIVANJE I UREĐIVANJE poslova*/
void ACO(int N){

	inicijalizacija_poslova(N);
	inicijalizacija_feromonskog_traga(N);

	//inicijalizacija sljedećeg posla
	vector<int> pom1;
	pom1.push_back(0);	//koji posao
	pom1.push_back(0);	//koji stroj
	sljedeca_aktivnost.clear();
	for(int i=0;i<broj_mrava;i++){
		sljedeca_aktivnost.push_back(pom1);
	}

	vector<vector<int> > pom2;
	redoslijed_obilaska.clear();
	for(int i=0;i<broj_mrava;i++){
		for(int j=0;j<vektor_poslova[N];j++){
			pom2.push_back(pom1);
		}
		redoslijed_obilaska.push_back(pom2);
		pom2.clear();
	}


	//indeks mrava s najboljim rješenjem
	int b=0;

	//mogući parametri za zaustavljanje
	double najkrace_vrijeme_trajanja_projekta=100000000;
	//ovdje ce se nalaziti moguci Cmax ili preostala 3 parametra
	double najkrace_vrijeme_trajanja_projekta_ikada=100000000;


	//ponovi onoliko puta koliko je broj generacija
	//ACO ALGORITAM
	cout<<"Progress ";
	for(int i=0;i<broj_generacija_mrava;i++){
		//za svaku generaciju ispiši 
		cout<<".";
		//uspostavi put za svakog mrava u turi
		inicijalizacija_mrava(N);

		//za sve poslove
		for(int i=0;i<vektor_poslova[N];i++){

				if(i<vektor_poslova[N]-1){
					//za sve mrave
					for(int k=0;k<broj_mrava;k++){
						q=drand();
						//određivanje hoće li mrav odabirati odlazak na temelju vjerojatnosne funkcije ili na temelju naučenog			
						if(q<=q0){

							//vraća vektor -> [posao,stroj]
							sljedeca_aktivnost[k]=eksploatacija(k,N);
						}
						else{
							//vraća vektor -> [posao,stroj]
							sljedeca_aktivnost[k]=istrazivanje(k,N);
						}
						//upisi da je mrav posjetio taj posao
						tabu_lista[k][sljedeca_aktivnost[k][0]]=0;
						//upisi koju je aktivnost mrav posjetio u vektor redoslijed_obilaska[k][i]
						//sljedeca_aktivnost[k][0]+=1;
						//sljedeca_aktivnost[k][1]+=1;
						redoslijed_obilaska[k][i]=sljedeca_aktivnost[k]; 
						redoslijed_obilaska[k][i][0]++;
						redoslijed_obilaska[k][i][1]++;
					}
				}
				else{

					//za sve mrave dodaj na kraju aktivnost iz koje su krenuli
					for(int k=0;k<broj_mrava;k++){
						sljedeca_aktivnost[k]=pocetna_aktivnost_mrava[k];	
						//u listu posjecenih upisujem trenutnu aktivnost
						//sljedeca_aktivnost[k][0]+=1;
						//sljedeca_aktivnost[k][1]+=1;
						redoslijed_obilaska[k][i]=sljedeca_aktivnost[k]; 
						redoslijed_obilaska[k][i][0]++;
						redoslijed_obilaska[k][i][1]++;
					}
				}
				//ažuriraj tragove za svakog mrava
				for(int k=0;k<broj_mrava;k++){

					int x,y0,y1;

					//vektor [posao,stroj]
					x=trenutna_aktivnost_mrava[k][0];
					//vektor [posao,stroj]
					y0=sljedeca_aktivnost[k][0];
					y1=sljedeca_aktivnost[k][1];

					//ispari trag na putu
					jacina_feromonskog_traga[x][y0][y1]=(1-ro)*jacina_feromonskog_traga[x][y0][y1]+ro*tau;
					trenutna_aktivnost_mrava[k]=sljedeca_aktivnost[k];			
				}
		}
	
		//promatranje trajanja projekta za svakog mrava 
		//FITNESS FUNKCIJA
		for(int i=0;i<broj_mrava;i++){
			trajanje_projekta[i]=fitness(redoslijed_obilaska[i],1,N); 
		}

		//traženje najboljeg rješenja
		for(int k=0;k<broj_mrava;k++){

			//ako je trajanje projekta manje od inicijalne vrijednosti 
			if(najkrace_vrijeme_trajanja_projekta>=trajanje_projekta[k]){

				//najmanja vrijednost postaje najkrace_vrijeme_trajanja_projekta i zapamti index mava koji je nasao najbolje rjesenje
				najkrace_vrijeme_trajanja_projekta=trajanje_projekta[k];
				b=k;
				//zapamti tu najbolju rutu
				//AKO ZELIM PAMTITI RASPORED, TREBAM GA ISCITATI IZ REDOSLIJEDA OBILAZAKA
				najbolji_slijed_aktivnosti.clear();
				for(int i=0;i<vektor_poslova[N];i++){

					najbolji_slijed_aktivnosti.push_back(redoslijed_obilaska[b][i]);
				}
			}
		}
		//ažuriranje tragova
		for(int i=0;i<vektor_poslova[N];i++)
			for(int j=0;j<vektor_poslova[N];j++){
				for(int k=0;k<vektor_strojeva[N];k++){

					if(heuristicke_vrijednosti[i][j][k]!=Beskonacnost){

						//feromonski trag nadodaje samo NAJBOLJI mrav
						//time se postiže točnija konvergencija
						jacina_feromonskog_traga[i][j][k]=(1-ro)*jacina_feromonskog_traga[i][j][k]+ro/(double)najkrace_vrijeme_trajanja_projekta; 
					}
				}
			}
	
			//zapamti vrijednost najboljeg rješenja
			if(najkrace_vrijeme_trajanja_projekta_ikada>najkrace_vrijeme_trajanja_projekta){

				najkrace_vrijeme_trajanja_projekta_ikada=najkrace_vrijeme_trajanja_projekta;
			}							
	}
	cout<<endl;
	cout<<"ACO je sve poslove rasporedio odjednom. Ne radi se izvrsavanje na zahtjev."<<endl;
}

/********************************************ACO********************************************/

/********************************************ACO NA ZAHTJEV********************************************/
/*Metoad za provjeru je li posao J dio Jmeta*/
int partOf(int j, vector<posao> Jmeta){

	for(int i=0;i<Jmeta.size();i++)
		if((Jmeta[i].brposla-1)==j)
			return 1;

	return 0;
}

/*Resetiraju se vrijednosti za feromosnki trag, a heuristika ostaje ista*/
void osvjezi_feromone(vector<posao> Jmeta, int N){

	//HEURISTIKA SE NE OSVJEŽAVA; ONA OSTAJE NEPROMJENJENA KROZ RAD U PROGRAMU
	//resetiram feromonski trag na inicijalne vrijednosti
	for(int i=0;i<vektor_poslova[N];i++){
		for(int j=0;j<vektor_poslova[N];j++){
			for(int k=0;k<vektor_strojeva[N];k++){
				if(i!=j)
					jacina_feromonskog_traga[i][j][k]=tau;
				else
					jacina_feromonskog_traga[i][j][k]=0;
			}
		}
	}
			
	//određivanje početnog položaja mrava
	//mravi se raspoređuju kružno, po modulu aktivnosti
	pocetna_aktivnost_mrava.clear();
	vector<int> pom3;
	int mche=-1;
	int mch;
	for(int i=0;i<broj_mrava;i++){
		int jb=i%Jmeta.size();
		if (jb==0){
			mche++;
			mch=mche%vektor_strojeva[N];
		}
		pom3.push_back(Jmeta[jb].brposla-1);
		pom3.push_back(mch);
		pocetna_aktivnost_mrava.push_back(pom3);
		pom3.clear();
	}
}

/*Obzirom na poslove u Jmeta, mrave raspoređuje u aktivna područja*/
void inicijalizacija_mrava_zahtjev(int N, vector<posao> Jmeta){

	vector<int> pom1;
	
	//resetiram tabu listu jer se ucestalo koristi
	//inicijalno u listu svim poslovima koji su ujedno dio Jmeta upisujem 1, a koji nisu, upisujem 0
	tabu_lista.clear();
	for(int i=0;i<broj_mrava;i++){
		for(int j=0;j<vektor_poslova[N];j++){
			if(partOf(j,Jmeta)){
				pom1.push_back(1);
			}else{
				pom1.push_back(0);
			}
		}
		tabu_lista.push_back(pom1);
		pom1.clear();
	}

	//svim mravima inicijalno upisujem u trajanje projekta 0 -> to je Cmax ili neki drugi parametar koji vraća fitness fja

	trajanje_projekta.clear();
    for(int i=0;i<broj_mrava;i++)
	   trajanje_projekta.push_back(0);

	trenutna_aktivnost_mrava.clear();

	//upisujem u tabu listu na mjesto aktivnosti u kojoj se mrav nalazi 0
	//u trenutnu aktivnost mrava inicijalno stavljam početnu vrijednost
	for(int k=0;k<broj_mrava;k++){
		//mrav,posao
		tabu_lista[k][pocetna_aktivnost_mrava[k][0]]=0;
		trenutna_aktivnost_mrava.push_back(pocetna_aktivnost_mrava[k]);
	}
}

/*ACO algoritam za PRIDRUŽIVANJE I UREĐIVANJE NA ZAHTJEV*/
void ACO_na_zahtjev(int N){

	posao zamjena;
	int gama,delta;

	clock_t pocetaK, kraJ;
	double sumator=0;
	int brojraspored=0;

	//skup svih raspoloživih poslova koji još nisu pokrenuti, inicijalno je Jmeta prazan
	vector<posao> Jmeta;
	//poslovi koji su dolazeci
	vector<posao> dolazeci;
	//vektor bool koji stroj trenutno radi
	vector<int> zaposlen;
	//vektor za prećenje dodjele prioriteta na nekom stroju
	vector<int> prati_prioritete;
	//pripravnost strojeva, inicijalno je 0
	vector<int> pripr_strojeva;
	//vektor velicine JMETA.size() za ispis
	vector<int> Jsize;

	izlazRAS<<"ACO: ";

	//kopija poslova u dolazece
	for(int i=0;i<poslovi.size();i++){
		dolazeci.push_back(poslovi[i]);
	}

	prioritet_na_stroju_zahtjev.clear();
	job_prethodi.clear();
	//početna inicijalizacija prioriteta na strojevima, pripreme stroja
	for(int i=0;i<vektor_strojeva[N];i++){
		prati_prioritete.push_back(0);
		pripr_strojeva.push_back(0);
		zaposlen.push_back(0);
		prioritet_na_stroju_zahtjev.push_back(0);
		job_prethodi.push_back(0);
	}

	//sortiranje dolazecih poslova po pripravnosti, uzlazno
	for(int i=0;i<dolazeci.size();i++){
		for(int j=i+1;j<dolazeci.size();j++){
			if(dolazeci[i].Rpripravnost>dolazeci[j].Rpripravnost){
				zamjena=dolazeci[i];
				dolazeci[i]=dolazeci[j];
				dolazeci[j]=zamjena;
			}
		}
	}

	//trenutno vrijeme postaje pripravno vrijeme
	int tren_time=dolazeci[0].Rpripravnost;

	//oni poslovi koji su pripravni, stavi ih u Jmeta
	int uzi=0;
	while(uzi<dolazeci.size()){

		if(tren_time==dolazeci[uzi].Rpripravnost){
			Jmeta.push_back(dolazeci[uzi]);
			dolazeci.erase(dolazeci.begin()+uzi);
			uzi=0;
			continue;
		}
		uzi++;
	}

	//inicijalizacija općenitih matrica
	inicijalizacija_poslova(N);
	inicijalizacija_feromonskog_traga(N);

	//inicijalizacija sljedećeg posla
	vector<int> pom1;
	pom1.push_back(0);	//koji posao
	pom1.push_back(0);	//koji stroj
	sljedeca_aktivnost.clear();
	for(int i=0;i<broj_mrava;i++){
		sljedeca_aktivnost.push_back(pom1);
	}

	vector<vector<int> > pom2;
	redoslijed_obilaska.clear();
	for(int i=0;i<broj_mrava;i++){
		for(int j=0;j<vektor_poslova[N];j++){
			pom2.push_back(pom1);
		}
		redoslijed_obilaska.push_back(pom2);
		pom2.clear();
	}

	//VELIKA PETLJA
	//dok postoje dolazeći ili postoje neprocesuirani
	cout<<"Progress ";
	while((dolazeci.size()>0) || (Jmeta.size()>0) ){

		pocetaK=clock();
		Jsize.push_back(Jmeta.size());

		//resetiranje sljedece aktivnosti i redoslijeda obilaska
		for(int i=0;i<broj_mrava;i++){
			sljedeca_aktivnost[i][0]=0;
			sljedeca_aktivnost[i][1]=0;
		}
		for(int i=0;i<broj_mrava;i++){
			for(int j=0;j<vektor_poslova[N];j++){
				redoslijed_obilaska[i][j][0]=0;
				redoslijed_obilaska[i][j][1]=0;
			}
		}

		//metoda koja s obzirom na Jmeta osvjezava 
		osvjezi_feromone(Jmeta,N);																
		//indeks mrava s najboljim rješenjem
		int b=0;

		//mogući parametri za zaustavljanje
		double najkrace_vrijeme_trajanja_projekta=100000000;
		//ovdje ce se nalaziti moguci Cmax ili preostala 3 parametra
		double najkrace_vrijeme_trajanja_projekta_ikada=100000000;

		//ponovi onoliko puta koliko je broj generacija
		//ACO ALGORITAM
		cout<<".";
		for(int i=0;i<broj_generacija_mrava;i++){
			//uspostavi put za svakog mrava u turi u ovisnosti o Jmeta
			inicijalizacija_mrava_zahtjev(N,Jmeta);											

			//za sve poslove u Jmeta
			for(int i=0;i<Jmeta.size();i++){

				if(i<Jmeta.size()-1){
						//za sve mrave
						for(int k=0;k<broj_mrava;k++){
							q=drand();
							//određivanje hoće li mrav odabirati odlazak na temelju vjerojatnosne funkcije ili na temelju naučenog			
							if(q<=q0){

								//vraća vektor -> [posao,stroj]
								sljedeca_aktivnost[k]=eksploatacija(k,N);
							}
							else{
								//vraća vektor -> [posao,stroj]
								sljedeca_aktivnost[k]=istrazivanje(k,N);
							}
							//upisi da je mrav posjetio taj posao
							tabu_lista[k][sljedeca_aktivnost[k][0]]=0;
							//upisi koju je aktivnost mrav posjetio u vektor redoslijed_obilaska[k][i]
							redoslijed_obilaska[k][i]=sljedeca_aktivnost[k]; 
							redoslijed_obilaska[k][i][0]++;
							redoslijed_obilaska[k][i][1]++;
						}
					}
					else{

						//za sve mrave dodaj na kraju aktivnost iz koje su krenuli
						for(int k=0;k<broj_mrava;k++){
							sljedeca_aktivnost[k]=pocetna_aktivnost_mrava[k];	
							//u listu posjecenih upisujem trenutnu aktivnost
							redoslijed_obilaska[k][i]=sljedeca_aktivnost[k]; 
							redoslijed_obilaska[k][i][0]++;
							redoslijed_obilaska[k][i][1]++;
						}
					}
					//ažuriraj tragove za svakog mrava
					for(int k=0;k<broj_mrava;k++){

						int x,y0,y1;

						//vektor [posao,stroj]
						x=trenutna_aktivnost_mrava[k][0];
						//vektor [posao,stroj]
						y0=sljedeca_aktivnost[k][0];
						y1=sljedeca_aktivnost[k][1];

						//ispari trag na putu
						jacina_feromonskog_traga[x][y0][y1]=(1-ro)*jacina_feromonskog_traga[x][y0][y1]+ro*tau;
						trenutna_aktivnost_mrava[k]=sljedeca_aktivnost[k];	
					}
			}
	
			//promatranje trajanja projekta za svakog mrava 
			//FITNESS FUNKCIJA
			for(int i=0;i<broj_mrava;i++){
				trajanje_projekta[i]=fitness(redoslijed_obilaska[i],-1,N);										
			}

			//traženje najboljeg rješenja
			for(int k=0;k<broj_mrava;k++){

				//ako je trajanje projekta manje od inicijalne vrijednosti 
				if(najkrace_vrijeme_trajanja_projekta>=trajanje_projekta[k]){

					//najmanja vrijednost postaje najkrace_vrijeme_trajanja_projekta i zapamti index mava koji je nasao najbolje rjesenje
					najkrace_vrijeme_trajanja_projekta=trajanje_projekta[k];
					b=k;
					//zapamti tu najbolju rutu
					//AKO ZELIM PAMTITI RASPORED, TREBAM GA ISCITATI IZ REDOSLIJEDA OBILAZAKA
					najbolji_slijed_aktivnosti.clear();
					for(int i=0;i<Jmeta.size();i++){

						najbolji_slijed_aktivnosti.push_back(redoslijed_obilaska[b][i]);
					}
				}
			}
			//ažuriranje tragova
			for(int i=0;i<vektor_poslova[N];i++)
				for(int j=0;j<vektor_poslova[N];j++){
					for(int k=0;k<vektor_strojeva[N];k++){

						if(heuristicke_vrijednosti[i][j][k]!=Beskonacnost){

							//feromonski trag nadodaje samo NAJBOLJI mrav
							//time se postiže točnija konvergencija
							jacina_feromonskog_traga[i][j][k]=(1-ro)*jacina_feromonskog_traga[i][j][k]+ro/(double)najkrace_vrijeme_trajanja_projekta; 
						}
					}
				}
				
			//zapamti vrijednost najboljeg rješenja
			if(najkrace_vrijeme_trajanja_projekta_ikada>najkrace_vrijeme_trajanja_projekta){

				najkrace_vrijeme_trajanja_projekta_ikada=najkrace_vrijeme_trajanja_projekta;
			}							
		} //kraj ACO algoritma

		//postoji li koji slobodan stroj za obradu poslova i koji je to
		for(int i=0;i<vektor_strojeva[N];i++){
			int pokrecem_posao=32000;
			//ako je stroj slobodan
			if((zaposlen[i]==0)&&(najbolji_slijed_aktivnosti.size()!=0)){
				//pronađi posao čiji prioritet izvođenja na tom stroju je najmanji
				for(int j=0;j<najbolji_slijed_aktivnosti.size();j++){
					if(najbolji_slijed_aktivnosti[j][1]-1==i){
						pokrecem_posao=j;
						//povećaj prioritet izvođenja na tom stroju
						prati_prioritete[i]++;;
						//odmah pokreni 1. u nizu najbolji_slijed_aktivnosti;
						break;
					}
				}

				if(pokrecem_posao==32000)
					continue;

				//kad je posao pronađen, on se pokreće na tom stroju, odnosno, briše se posao iz najbolji_slijed_aktivnosti, a povećava se vrijeme priprave stroja i za trajanje posla j
				for(int k=0;k<poslovi.size();k++){
					if(poslovi[k].brposla==najbolji_slijed_aktivnosti[pokrecem_posao][0]){
						//upisivanje podataka u posao
						poslovi[k].rb_stroj=najbolji_slijed_aktivnosti[pokrecem_posao][1];	
						poslovi[k].prioritet=prati_prioritete[najbolji_slijed_aktivnosti[pokrecem_posao][1]-1];
						//zaposli stroj
						zaposlen[i]=1;
						//povecaj vrijeme pripravnosti stroja na kojem se izvrsava obrada
						pripr_strojeva[i]=poslovi[k].trajanje[i];
						//izbaci taj posao iz Jmeta
						for(int z=0;z<Jmeta.size();z++){
							if(Jmeta[z].brposla==najbolji_slijed_aktivnosti[pokrecem_posao][0]){
								//cout<<"Iz Jmeta brisem posao "<<Jmeta[z].brposla<<endl;
								Jmeta.erase(Jmeta.begin()+z);
							}
						}
						//posto je posao pokrenut, izbaci ga iz najbolji_slijed_aktivnosti
						najbolji_slijed_aktivnosti.erase(najbolji_slijed_aktivnosti.begin()+pokrecem_posao);
						break;
					}
				}
			}
		}

		//sad trebam imati 3 informacije: 1.trenutno vrijeme  2.najmanja priprava strojeva  3.najmanja priprava poslova
		//te 3 informacije kombinira funkcija: racunaj_vrijeme
		
		//vraca vrijednost najmanje obrade na svim strojevima
		gama=racunaj_ro(pripr_strojeva);
		//vraca vrijeme potrebno da bi sljedeći posao postao pripravan iz dolazećih
		delta=racunaj_delta(dolazeci,tren_time);

		//ako je vrijeme za pripravu sljedećeg posla manje ili jednako vremenu obrade posla na nekom od strojeva
		if((gama<=delta) || (delta==0)){
			for(int i=0;i<vektor_strojeva[N];i++){
				//oduzmi trajanje procesiranja na tom stroju
				if(pripr_strojeva[i]!=0){
					pripr_strojeva[i]-=gama;
					//ako je vrijednost pripravnosti stroja jednaka 0, onda je taj stroj odmah spreman za posao
					if(pripr_strojeva[i]==0){
						zaposlen[i]=0;
					}
				}
			}
		}else{
			//ako je vrijeme dolaska u sustav manje od obrade
			for(int i=0;i<vektor_strojeva[N];i++){
				if(pripr_strojeva[i]!=0)
					pripr_strojeva[i]-=delta;
			}
		}

		//uvećaj trenutno vrijeme
		if(delta!=0)
			tren_time+=delta;
		else
			tren_time+=gama;

		//pokreni sve poslove koji mogu početi
		int ii=0;
		while(ii<dolazeci.size()){

			if (tren_time<dolazeci[ii].Rpripravnost)
				break;

			if(tren_time==dolazeci[ii].Rpripravnost){
				Jmeta.push_back(dolazeci[ii]);
				dolazeci.erase(dolazeci.begin()+ii);
				ii=0;
				continue;
			}
			ii++;
		}

		kraJ=clock();
		izlazRAS<<(double(kraJ-pocetaK)/(CLOCKS_PER_SEC/1000))<<" ";
		sumator+=(double(kraJ-pocetaK)/(CLOCKS_PER_SEC/1000));
		brojraspored++;
	}
	izlazRAS<<endl;
	izlazRAS<<"JMETA[ACO]: ";
	for (int i=0;i<Jsize.size();i++){
		izlazRAS<<Jsize[i]<<" ";
	}
	cout<<endl;
	cout<<"Prosjecno trajanje jednog rasporedivanja algoritma ACO je "<<(double)(sumator/brojraspored)<<" ms."<<endl;
	izracunaj_prethodnika(N);
	cout<<endl;
}

/********************************************ACO NA ZAHTJEV********************************************/

/*Ucitavanje ulaznih podataka; većinom za ACO*/
void ucitaj_podatke_za_program(){

	int izbor, izbor1;

	cout<<"Odaberite jednu od ponudjenih mogucnosti..."<<endl;
	cout<<endl;
	cout<<"		1] Predodredeno rasporedivanje"<<endl;
	cout<<"		2] Rasporedivanje na zahtjev"<<endl;
	cout<<"		Odgovor: ";
	cin>>tip_rasporedivanja;										//->
	cout<<endl;
	cout<<"Odaberite kriterij optimizacije..."<<endl;
	cout<<endl;
	cout<<"		1] Cmax, duljina rasporeda"<<endl;
	cout<<"		2] Fw, tezinsko protjecanje"<<endl;
	cout<<"		3] Tw, tezinsko zaostajanje"<<endl;
	cout<<"		4] Uw, tezinska zakasnjelost"<<endl;
	cout<<"		Odgovor: ";
	cin>>kriterij_optimizacije;

	if(kriterij_optimizacije==1){
		ime_dominacija_dat="dominacija_Cmax.txt";
	}else if(kriterij_optimizacije==2){
		ime_dominacija_dat="dominacija_Fw.txt";
	}else if(kriterij_optimizacije==3){
		ime_dominacija_dat="dominacija_Tw.txt";
	}else{
		ime_dominacija_dat="dominacija_Uw.txt";
	}

	cout<<endl;
	
	if(tip_rasporedivanja==1){

		ime_izlaz_dat="predodredeno_ACO.txt";

		cout<<endl;
		cout<<"Unesite pocetni indeks skupa poslova za analizu (prvi je 0): ";
		cin>>pocetni_index;
		cout<<"Unesite zavrsni indeks skupa poslova za analizu (posljednji je 120): ";
		cin>>zavrsni_index;
		cout<<"Zelite li pretpostavljene vrijednosti u programu ili zelite upisivati vrijednosti [1-da/0-ne]? ";
		cin>>izbor1;

		if(izbor1==0){
			cout<<"Unesite broj generacija mrava ($! in [1..100] bit ce skalirani unutar tog intervala): ";
			cin>>broj_generacija_mrava;
			broj_generacija_mrava=(broj_generacija_mrava%101)+1;
			cout<<"Unesite ALFA vrijednost: ";
			cin>>alfa;
			cout<<"Unesite BETA vrijednost: ";
			cin>>beta;
			cout<<"Unesite TAU vrijednost: ";
			cin>>tau;
			cout<<"Unesite RO vrijednost: ";
			cin>>ro;
			cout<<"Broj mrava inicijalno se postavlja na vrijednost: JxM (broj_poslova x broj_strojeva). Taj broj oznacen je vrijednoscu N."<<endl;
			cout<<"Unesite broj mrava..."<<endl;
			cout<<"		1] N"<<endl;
			cout<<"		2] N/2"<<endl;
			cout<<"		3] N/3"<<endl;
			cout<<"		4] N/4"<<endl;
			cout<<"		Odgovor: ";
			cin>>izbor;
			switch(izbor){

				case 1:	
					koef_mravi=1;
					break;

				case 2:	
					koef_mravi=0.5;	
					break;

				case 3:	
					koef_mravi=0.3;	
					break;

				case 4: 
					koef_mravi=0.25;	
					break;
			}
		}else{
			cout<<"Upisane su pretpostavljene vrijednosti..."<<endl;
			broj_generacija_mrava=40;
			alfa=1.1;
			beta=1.3;
			tau=0.5;
			ro=0.5;
			koef_mravi=0.5;
			cout<<"Broj generacija mrava="<<broj_generacija_mrava<<"|| Alfa="<<alfa<<"|| Beta="<<beta<<"|| Tau="<<tau<<"|| Ro="<<ro<<"|| Broj mrava=N*"<<koef_mravi<<endl;
		}

	}else if(tip_rasporedivanja==2){

		ime_izlaz_dat="na_zahtjev_ACO.txt";

		cout<<endl;
		cout<<"Unesite pocetni indeks skupa poslova za analizu (prvi je 0): ";
		cin>>pocetni_index;
		cout<<"Unesite zavrsni indeks skupa poslova za analizu (posljednji je 120): ";
		cin>>zavrsni_index;
		cout<<"Zelite li pretpostavljene vrijednosti u programu ili zelite upisivati vrijednosti [1-da/0-ne]? ";
		cin>>izbor1;

		if(izbor1==0){
			cout<<"Unesite broj generacija mrava*	($! in [1..5] bit ce skalirani unutar tog intervala): ";
			cin>>broj_generacija_mrava;
			broj_generacija_mrava=(broj_generacija_mrava%6)+1;
			cout<<"Unesite ALFA vrijednost: ";
			cin>>alfa;
			cout<<"Unesite BETA vrijednost: ";
			cin>>beta;
			cout<<"Unesite TAU vrijednost: ";
			cin>>tau;
			cout<<"Unesite RO vrijednost: ";
			cin>>ro;
			cout<<"Broj mrava inicijalno se postavlja na vrijednost: JxM (broj_poslova x broj_strojeva). Taj broj oznacen je vrijednoscu N."<<endl;
			cout<<"Unesite broj mrava..."<<endl;
			cout<<"		1] N"<<endl;
			cout<<"		2] N/2"<<endl;
			cout<<"		3] N/3"<<endl;
			cout<<"		4] N/4"<<endl;
			cout<<"		Odgovor: ";
			cin>>izbor;
			switch(izbor){

				case 1:	
					koef_mravi=1;
					break;

				case 2:	
					koef_mravi=0.5;	
					break;

				case 3:	
					koef_mravi=0.3;	
					break;

				case 4: 
					koef_mravi=0.25;	
					break;
			}
		}else{
			cout<<"Upisane su pretpostavljene vrijednosti..."<<endl;
			broj_generacija_mrava=2;
			alfa=1.1;
			beta=1.3;
			tau=0.5;
			ro=0.5;
			koef_mravi=0.5;
			cout<<"Broj generacija mrava="<<broj_generacija_mrava<<"|| Alfa="<<alfa<<"|| Beta="<<beta<<"|| Tau="<<tau<<"|| Ro="<<ro<<"|| Broj mrava=N*"<<koef_mravi<<endl;
		}
	}else{

		cout<<"Unjeli ste nedozvoljenu vrijednost. Program se prekida..."<<endl;
		exit(1);
	}
}

void ispisi_naslov(){

	cout<<"FAKULTET ELEKTROTEHNIKE I RACUNARSTVA	||	SVEUCILISTE U ZAGREBU	|| ZEMRIS"<<endl;
	cout<<"Student:	Karlo Knezevic, 0036443568, Racunarska znanost"<<endl;
	cout<<"Mentor:	doc.dr.sc. Domagoj Jakobovic, ZEMRIS"<<endl;
	cout<<"______________________________________________________________________________"<<endl;

}

//Main metoda
int main(){

	clock_t start, finish, start_p, finish_p;
	vector<vector<int> > np;

	ispisi_naslov();

	ucitaj_podatke_za_program();

	ofstream izlaz(ime_izlaz_dat.c_str());
	ofstream izlazMIN("min_min.txt");
	ofstream izlazDOM(ime_dominacija_dat.c_str());

	//početak mjerenja obrade programa 
	start=clock();

	//citanje podataka o kolicini poslova, strojeva
	citanje_glavne_datoteke();
	cout<<endl;

	//ULAZ U PROGRAM
	for(int i=pocetni_index;i<zavrsni_index;i++){

		broj_mrava=floor(koef_mravi*vektor_poslova[i]*vektor_strojeva[i]);
		//stvaranje vektora za poslove
		inicijaliziraj_vektor_poslova(i);
		//upisivanje pripravnosti u poslove
		citanje_dolazaka_poslova(i);
		//upisivanje trajanja u poslove
		citanje_trajanja_poslova(i);
		//upisivanje zavrsetaka trajanja
		citanje_zavrsetaka_poslova(i);
		//upisivanje tezinskih faktora trajanja
		citanje_tezinskih_faktora_poslova(i);

		if(tip_rasporedivanja==1){

			cout<<"Procitane informacije za "<<i+1<<". skup:  "<<endl;

			cout<<"[MIN-MIN]"<<endl;

			//predodređeno raspoređivanje
			start_p=clock();
			min_min(i);
			finish_p=clock();
			izlazRAS<<endl;

			fitness(np,0,i);

			cout<<endl;

			cout<<"Cmax= "<<dobrota_rasporeda.Cnaj<<" ||  Fw= "<<dobrota_rasporeda.Fnaj<<" ||  Tw= "<<dobrota_rasporeda.Tnaj<<" ||  Uw= "<<dobrota_rasporeda.Unaj<<endl;
			cout<<"Trajanje izvodjenja: "<<(double(finish_p-start_p)/CLOCKS_PER_SEC)<<" sec."<<endl;
			izlazMIN<<"MM Cmax= "<<dobrota_rasporeda.Cnaj<<" ||  Fw= "<<dobrota_rasporeda.Fnaj<<" ||  Tw= "<<dobrota_rasporeda.Tnaj<<" ||  Uw= "<<dobrota_rasporeda.Unaj<<"  || "<<"Trajanje izvodjenja: "<<(double(finish_p-start_p)/CLOCKS_PER_SEC)<<endl;

			dominacija_podaci.Cduljina=dobrota_rasporeda.Cnaj;
			dominacija_podaci.Fprotjecanje=dobrota_rasporeda.Fnaj;
			dominacija_podaci.Tzaostajanje=dobrota_rasporeda.Tnaj;
			dominacija_podaci.Uzakasnjelost=dobrota_rasporeda.Unaj;

			cout<<":: :: :: :: ::"<<endl;

			cout<<"[ACO]"<<endl;

			start_p=clock();
			ACO(i);
			finish_p=clock();

			fitness(najbolji_slijed_aktivnosti,1,i);

			cout<<"ACO Cmax= "<<dobrota_rasporeda.Cnaj<<" ||  Fw= "<<dobrota_rasporeda.Fnaj<<" ||  Tw= "<<dobrota_rasporeda.Tnaj<<" ||  Uw= "<<dobrota_rasporeda.Unaj<<endl;
			cout<<"Trajanje izvodjenja: "<<(double(finish_p-start_p)/CLOCKS_PER_SEC)<<" sec."<<endl;
			izlaz<<"ACO Cmax= "<<dobrota_rasporeda.Cnaj<<" ||  Fw= "<<dobrota_rasporeda.Fnaj<<" ||  Tw= "<<dobrota_rasporeda.Tnaj<<" ||  Uw= "<<dobrota_rasporeda.Unaj<<"  || "<<"Trajanje izvodjenja: "<<(double(finish_p-start_p)/CLOCKS_PER_SEC)<<endl;

			//ako je ACO bolji ili jednak, upisujem 1, u suprotnom 0
			if(dominacija_podaci.Cduljina>dobrota_rasporeda.Cnaj){
				izlazDOM<<"Cmax ACO= "<<1<<" MM= "<<0<<" || ";
			}else if(dominacija_podaci.Cduljina<dobrota_rasporeda.Cnaj){
				izlazDOM<<"Cmax ACO= "<<0<<" MM= "<<1<<" || ";
			}else
				izlazDOM<<"Cmax ACO= "<<1<<" MM= "<<1<<" || ";

			if(dominacija_podaci.Fprotjecanje>dobrota_rasporeda.Fnaj){
				izlazDOM<<"Fw ACO= "<<1<<" MM= "<<0<<" || ";
			}else if(dominacija_podaci.Fprotjecanje<dobrota_rasporeda.Fnaj){
				izlazDOM<<"Fw ACO= "<<0<<" MM= "<<1<<" || ";
			}else
				izlazDOM<<"Fw ACO= "<<1<<" MM= "<<1<<" || ";

			if(dominacija_podaci.Tzaostajanje>dobrota_rasporeda.Tnaj){
				izlazDOM<<"Tw ACO= "<<1<<" MM= "<<0<<" || ";
			}else if(dominacija_podaci.Tzaostajanje<dobrota_rasporeda.Tnaj){
				izlazDOM<<"Tw ACO= "<<0<<" MM= "<<1<<" || ";
			}else
				izlazDOM<<"Tw ACO= "<<1<<" MM= "<<1<<" || ";

			if(dominacija_podaci.Uzakasnjelost>dobrota_rasporeda.Unaj){
				izlazDOM<<"Uw ACO= "<<1<<" MM= "<<0<<endl;
			}else if(dominacija_podaci.Uzakasnjelost<dobrota_rasporeda.Unaj){
				izlazDOM<<"Uw ACO= "<<0<<" MM= "<<1<<endl;
			}else
				izlazDOM<<"Uw ACO= "<<1<<" MM= "<<1<<endl;

			cout<<"...	...	...	...	...	...	...	...	"<<endl;

			cout<<endl;
			//unisti ucitani vektor
			poslovi.clear();


		}else{

			//raspoređivanje na zahtjev
			cout<<"Procitane informacije za "<<i+1<<". skup:  "<<endl;

			cout<<"[MIN-MIN]"<<endl;

			start_p=clock();
			min_min(i);
			finish_p=clock();
			izlazRAS<<endl;

			fitness(np,0,i);

			cout<<endl;


			cout<<"Cmax= "<<dobrota_rasporeda.Cnaj<<" ||  Fw= "<<dobrota_rasporeda.Fnaj<<" ||  Tw= "<<dobrota_rasporeda.Tnaj<<" ||  Uw= "<<dobrota_rasporeda.Unaj<<endl;
			cout<<"Trajanje izvodjenja: "<<(double(finish_p-start_p)/CLOCKS_PER_SEC)<<" sec."<<endl;
			izlazMIN<<"MM Cmax= "<<dobrota_rasporeda.Cnaj<<" ||  Fw= "<<dobrota_rasporeda.Fnaj<<" ||  Tw= "<<dobrota_rasporeda.Tnaj<<" ||  Uw= "<<dobrota_rasporeda.Unaj<<"  || "<<"Trajanje izvodjenja: "<<(double(finish_p-start_p)/CLOCKS_PER_SEC)<<endl;

			dominacija_podaci.Cduljina=dobrota_rasporeda.Cnaj;
			dominacija_podaci.Fprotjecanje=dobrota_rasporeda.Fnaj;
			dominacija_podaci.Tzaostajanje=dobrota_rasporeda.Tnaj;
			dominacija_podaci.Uzakasnjelost=dobrota_rasporeda.Unaj;

			cout<<":: :: :: :: ::"<<endl;

			cout<<"[ACO na zahtjev]"<<endl;

			start_p=clock();
			ACO_na_zahtjev(i);
			finish_p=clock();
			izlazRAS<<endl;

			/*cout<<"Raspored poslova na strojeve [ACO]: ";
			for(int j=0;j<poslovi.size();j++){
				cout<<poslovi[j].rb_stroj<<"  ";
			}
			cout<<endl;*/

			fitness(np,0,i);

			cout<<"Cmax= "<<dobrota_rasporeda.Cnaj<<" ||  Fw= "<<dobrota_rasporeda.Fnaj<<" ||  Tw= "<<dobrota_rasporeda.Tnaj<<" ||  Uw= "<<dobrota_rasporeda.Unaj<<endl;
			cout<<"Trajanje izvodjenja: "<<(double(finish_p-start_p)/CLOCKS_PER_SEC)<<" sec."<<endl;
			izlaz<<"ACO Cmax= "<<dobrota_rasporeda.Cnaj<<" ||  Fw= "<<dobrota_rasporeda.Fnaj<<" ||  Tw= "<<dobrota_rasporeda.Tnaj<<" ||  Uw= "<<dobrota_rasporeda.Unaj<<"  || "<<"Trajanje izvodjenja: "<<(double(finish_p-start_p)/CLOCKS_PER_SEC)<<endl;

			//ako je ACO bolji ili jednak, upisujem 1, u suprotnom 0
			if(dominacija_podaci.Cduljina>dobrota_rasporeda.Cnaj){
				izlazDOM<<"Cmax ACO= "<<1<<" MM= "<<0<<" || ";
			}else if(dominacija_podaci.Cduljina<dobrota_rasporeda.Cnaj){
				izlazDOM<<"Cmax ACO= "<<0<<" MM= "<<1<<" || ";
			}else
				izlazDOM<<"Cmax ACO= "<<1<<" MM= "<<1<<" || ";

			if(dominacija_podaci.Fprotjecanje>dobrota_rasporeda.Fnaj){
				izlazDOM<<"Fw ACO= "<<1<<" MM= "<<0<<" || ";
			}else if(dominacija_podaci.Fprotjecanje<dobrota_rasporeda.Fnaj){
				izlazDOM<<"Fw ACO= "<<0<<" MM= "<<1<<" || ";
			}else
				izlazDOM<<"Fw ACO= "<<1<<" MM= "<<1<<" || ";

			if(dominacija_podaci.Tzaostajanje>dobrota_rasporeda.Tnaj){
				izlazDOM<<"Tw ACO= "<<1<<" MM= "<<0<<" || ";
			}else if(dominacija_podaci.Tzaostajanje<dobrota_rasporeda.Tnaj){
				izlazDOM<<"Tw ACO= "<<0<<" MM= "<<1<<" || ";
			}else
				izlazDOM<<"Tw ACO= "<<1<<" MM= "<<1<<" || ";

			if(dominacija_podaci.Uzakasnjelost>dobrota_rasporeda.Unaj){
				izlazDOM<<"Uw ACO= "<<1<<" MM= "<<0<<endl;
			}else if(dominacija_podaci.Uzakasnjelost<dobrota_rasporeda.Unaj){
				izlazDOM<<"Uw ACO= "<<0<<" MM= "<<1<<endl;
			}else
				izlazDOM<<"Uw ACO= "<<1<<" MM= "<<1<<endl;

			cout<<"...	...	...	...	...	...	...	...	"<<endl;

			cout<<endl;
			//unisti ucitani vektor
			poslovi.clear();

		}
	}

	finish=clock();
	cout<<"Program je trajao "<<(double(finish-start)/CLOCKS_PER_SEC)<<" sec."<<endl;
	system("PAUSE");
}