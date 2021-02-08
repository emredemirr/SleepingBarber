#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

//Semafor Kilitlerinin Tanımlanması
sem_t berberler;
sem_t musteriler;
sem_t mutex;

int BerberKoltuguSayisi = 1;
int MusteriKoltuguSayisi = 5;
int BosMusteriKoltuguSayisi = 0;
int Musterisayisi = 0;
int ToplamHizmetVerilmisMusteri = 0;
int sandalye = 0;
int *BerberKoltugu;

void Berber(void *berberID){
	int sayi=*(int*)berberID+1; //Berberin ID sinin tanımlanması
	int musID, sonrakiMusteri;

	printf("Berber %d dükkana geldi.\n",sayi);

	while(1){
		//Müşteri yoksa berber uyumaya gider.
		if(!musID)
			printf("Berber %d uyudu.\n\n",sayi);
		sem_wait(&berberler); //Boşta berber varsa berber koltuguna oturmak icin izin ister.
		sem_wait(&mutex); //berber koltuğuna aynı anda başka müşterinin oturmasını engeller. 

		//Bekleyenlerin arasından müşteri seçilir
		ToplamHizmetVerilmisMusteri= (++ToplamHizmetVerilmisMusteri) % MusteriKoltuguSayisi;
		sonrakiMusteri = ToplamHizmetVerilmisMusteri;
		musID = BerberKoltugu[sonrakiMusteri];
		BerberKoltugu[sonrakiMusteri] = pthread_self();

		sem_post(&mutex); //Koltuğun kilidinin açılması
		sem_post(&musteriler); //Koltugun bosaldigini belirtir

		printf("Berber %d müşteri %d 'ye hizmet vermeye başladı.\n\n",sayi,musID);
		sleep(1);
		printf("Berber %d müşteri %d 'ye hizmet vermeyi bitirdi.\n\n",sayi,musID);

		if(MusteriKoltuguSayisi == BosMusteriKoltuguSayisi){ //Bekleyen müşteri yoksa berber uyur
			printf("Berber %d uyudu.\n\n",sayi);
		}
	}

	pthread_exit(0); // İş parçacığının bitmesini bekler
}

void Musteri (void *musteriID){

	int sayi = *(int*)musteriID+1;//Musterinin ID sinin tanımlanması
	int oturulanSandalye, berID;

	sem_wait(&mutex); //Müşteri dükkana girer.
	printf("Müşteri %d dükkana geldi.\n",sayi);

	//Dükkanda boş müşteri koltuğunun olup olmadığının kontrol edilmesi
	if(BosMusteriKoltuguSayisi >0){
		BosMusteriKoltuguSayisi--; //Müşteri dükkana girdiği için boş müşteri koltuğu sayısı 1 azaltılır

		printf("Müşteri %d bekliyor.\n\n",sayi);

		//müşteri koltuklarından birine oturur
		sandalye = (++sandalye) % MusteriKoltuguSayisi;
		oturulanSandalye = sandalye ;
		BerberKoltugu[oturulanSandalye]=sayi;

		sem_post(&mutex); //Müşterinin koltuktan kalkıp koltuk üzerindeki kilidin kaldırılması
		sem_post(&berberler); //Uyuyan berberi uyandırır.

		sem_wait(&musteriler); //Müşterinin berber koltuğu boşsa oturmak için izin ister.
		sem_wait(&mutex); //Müşteri aynı anda başka bir müşterinin berber koltuğuna oturmasını engellemek için koltuğu kilitler.

		berID = BerberKoltugu[oturulanSandalye];
		BosMusteriKoltuguSayisi++;//Müşteri müşteri koltuklarından berber koltuğuna geçtiği için boş koltuk sayısı arttırılır

		sem_post(&mutex);//Müşterinin berber koltuğundan ayrılması
	}
	else {
		//Dükkanda boş yer yoksa müşteri dükkandan ayrılır
		sem_post(&mutex);
		printf("Berber dükkanı dolu, müşteri %d dükkandan ayrılıyor.\n\n",sayi);
	}

	pthread_exit(0);
}

int main (int argc){
	//Kullanıcılardan verilerin alınması
	printf("Müşteri Sayısını Girin : ");
	scanf("%d",&Musterisayisi);

	BosMusteriKoltuguSayisi=MusteriKoltuguSayisi;  //Boş müşteri koltuklarının sayısının belirlenmesi
	BerberKoltugu=(int*) malloc(sizeof(int) * MusteriKoltuguSayisi);  //Berber koltuğu dizisinin oluşturulması

	int berberIDleri[BerberKoltuguSayisi]; //Berber koltuğu kadar berberin tanımlanması

	pthread_t berber[BerberKoltuguSayisi]; //Berber threadlerinin tanımlanması
	pthread_t musteri[Musterisayisi]; //Müşteri threadlerinin tanımlanması

	//Semafor kilitlerinin ilk değerlerinin atanması
	sem_init(&berberler, 0, 0);
	sem_init(&musteriler, 0, 0);
	sem_init(&mutex, 0, 1);

	printf("\nBerber dükkanı açıldı.\n\n");

	//berber threadlerinin oluşturulması
	int i;
	for(i=0; i<BerberKoltuguSayisi; i++)
	{
		pthread_create(&berber[i], NULL, (void*)Berber, (void*)&i);
		sleep(1); 
	}

	//Müşteri threadlerinin oluşturulması
	int j;
	for(j=0; j<Musterisayisi; j++)
	{
		pthread_create(&musteri[j], NULL, (void*)Musteri, (void*)&j);
		srand((unsigned int)time(NULL)); //Müşterilerin rastgele zamanlarda gelmeleri için random değerlerin atanması
		usleep(rand() % (250000 - 50000 +1 ) +50000); 

	}

	//Müşteri threadlerinin bitirilmesi
	int k;
	for(k=0; k<Musterisayisi; k++)
	{
		pthread_join(musteri[k],NULL);
	}

	sleep(1);

	//Semaforların kaldırılması
	sem_destroy(&berberler);
	sem_destroy(&musteriler);
	sem_destroy(&mutex);

	printf("Berber dükkanı kapatılacak...\n\n");

	return 0;

}
