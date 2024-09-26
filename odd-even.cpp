#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <mpi.h>

using namespace std;

void oddEvenOwn(vector<int> & sl){
    bool desordenado = 1;
    while(desordenado){
        desordenado = 0;
        for(int i = 0; i < sl.size(); i++){
            if(i%2 != 0) {
                if(i < sl.size()-1){
                    if(sl[i] > sl[i+1]){
                        swap(sl[i], sl[i+1]);
                        desordenado = 1;
                    }
                }
            }  
        }

        for(int i = 0; i < sl.size(); i++){
            if(i%2 == 0) {
                if(i < sl.size()-1){
                    if(sl[i] > sl[i+1]){
                        swap(sl[i], sl[i+1]);
                        desordenado = 1;
                    }
                }
            }  
        }
    }
}

int Compute_partner(int phase, int my_rank, int comm_sz) {
    int partner;
    if (phase % 2 == 0) {
        if (my_rank % 2 != 0)
            partner = my_rank - 1;
        else
            partner = my_rank + 1;
    } else {
        if (my_rank % 2 != 0)
            partner = my_rank + 1;
        else
            partner = my_rank - 1;
    }

    if (partner == -1 || partner == comm_sz)
        partner = MPI_PROC_NULL;

    return partner;
}


void oddEven(vector<int> & sublista, int t, int id){
    oddEvenOwn(sublista);
    for(int i = 0; i < t; i++){
        int partner = Compute_partner(i, id, t);
        if(partner != MPI_PROC_NULL){
            if(id < partner){
                int size = sublista.size();
                MPI_Send(&size, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
                MPI_Send(sublista.data(), size, MPI_INT, partner, 0, MPI_COMM_WORLD);
                int tamano;
                MPI_Recv(&tamano, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                vector<int> lista2(tamano);
                MPI_Recv(lista2.data(), tamano, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                vector<int> vector_unido = sublista;
                vector_unido.insert(vector_unido.end(), lista2.begin(), lista2.end());
                // cout << id << " " << partner << endl;
                // for(int i = 0; i < lista2.size(); i++){
                //     cout << vector_unido[i] << " ";
                // }
                // cout << endl;
                oddEvenOwn(vector_unido);
                // for (int i = 0; i < size; i++) {
                //     cout << sublista[i] << " ";
                // }
                // cout << endl;
                // for (int i = 0; i < tamano; i++) {
                //     cout << lista2[i] << " ";
                // }
                // cout << endl;
                // cout << "Vector unido :" << endl;
                // for(int i = 0; i < vector_unido.size(); i++){
                //     cout << vector_unido[i] << " ";
                // }
                // cout << endl;
                sublista.clear();
                for(int i = 0; i < size; i++){
                    sublista.push_back(vector_unido[i]);
                }
            }
            else{
                int size = sublista.size();
                MPI_Send(&size, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
                MPI_Send(sublista.data(), size, MPI_INT, partner, 0, MPI_COMM_WORLD);
                int tamano;
                MPI_Recv(&tamano, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                vector<int> lista2(tamano);
                MPI_Recv(lista2.data(), tamano, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                vector<int> vector_unido = sublista;
                vector_unido.insert(vector_unido.end(), lista2.begin(), lista2.end());
                // cout << id << " " << partner << endl;
                // for(int i = 0; i < vector_unido.size(); i++){
                //     cout << vector_unido[i] << " ";
                // }
                // cout << endl;
                oddEvenOwn(vector_unido);
                // for (int i = 0; i < size; i++) {
                //     cout << sublista[i] << " ";
                // }
                // cout << endl;
                // for (int i = 0; i < tamano; i++) {
                //     cout << lista2[i] << " ";
                // }
                // cout << endl;
                // cout << "Vector unido :" << endl;
                // for(int i = 0; i < vector_unido.size(); i++){
                //     cout << vector_unido[i] << " ";
                // }
                // cout << endl;
                sublista.clear();
                for(int i = tamano; i < vector_unido.size(); i++){
                    sublista.push_back(vector_unido[i]);
                }
            }
        } 
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int tam, id;
    MPI_Comm_size(MPI_COMM_WORLD, &tam);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    int cantidad = 1000;
    int subcantidad = ceil(cantidad / tam);

    vector<int> lista(cantidad);
    vector<int> sublista(subcantidad);

    if (id == 0) {
        srand(time(0));
        for (int i = 0; i < cantidad; i++) {
            lista[i] = rand() % cantidad;
        }

        cout << "Lista original: ";
        for (int i = 0; i < cantidad; i++) {
            cout << lista[i] << " ";
        }
        cout << endl;
    }

    MPI_Scatter(lista.data(), subcantidad, MPI_INT, sublista.data(), subcantidad, MPI_INT, 0, MPI_COMM_WORLD);

    oddEvenOwn(lista);
    // cout << "Proceso " << id << " recibió: ";
    // for (int i = 0; i < subcantidad; i++) {
    //     cout << sublista[i] << " ";
    // }
    // cout << endl;


    oddEven(sublista, tam, id);
    
    MPI_Gather(sublista.data(), subcantidad, MPI_INT, lista.data(), subcantidad, MPI_INT, 0, MPI_COMM_WORLD);
    if(id == 0){
        cout << "Lista ordenada: ";
        for (int i = 0; i < cantidad; i++) {
            cout << lista[i] << " ";
        }
        cout << endl;
    }

    MPI_Finalize();
    return 0;
}
