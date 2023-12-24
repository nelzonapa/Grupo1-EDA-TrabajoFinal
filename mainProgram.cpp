#include "rstartree.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>


using namespace std;



struct PatientData
{
    vector<string> characteristics; // Para almacenar las características del paciente
    vector<string> dataSubset;      // Para almacenar un subconjunto específico de datos del paciente
};

ostream &operator<<(ostream &out, const PatientData &paciente)
{
    for (int i = 0; i < paciente.characteristics.size(); i++)
    {
        out << "|\t" << paciente.characteristics[i] << "|\t";
    }
    out << "\n";

    for (int i = 0; i < paciente.dataSubset.size(); i++)
    {
        out << "\t" << paciente.dataSubset[i] << "\t";
    }
    out << "\n";

    return out;
}

RStarBoundingBox<3> createBox3D(int testResult, double ageQuantile, double hemoglobin, 
double platelets, double meanPlateletVolume, double mchc, double leukocytes, 
double basophils, double eosinophils, double monocytes, double crp)
{
    RStarBoundingBox<3> box;  // Creamos la caja con tres tamaños diferentes

    // Estos valores representan los tamaños mínimos de la caja en cada dimensión
    box.min_edges[0] = ageQuantile;
    box.min_edges[1] = ageQuantile;
    box.min_edges[2] = hemoglobin;

    // Al principio, los tamaños máximos son iguales a los tamaños mínimos,
    // así que la caja tiene el mismo tamaño en todas las dimensiones por ahora
    box.max_edges[0] = ageQuantile;
    box.min_edges[1] = ageQuantile;
    box.max_edges[2] = hemoglobin;

    // Ahora, ajustaremos el tamaño de la caja en cada dimensión basándonos en los valores

    // Hematocrit
    box.min_edges[3] = platelets;
    box.max_edges[3] = platelets;

    // Mean platelet volume
    if (mchc < 0) {
        box.min_edges[4] = mchc;
    } else {
        box.max_edges[4] = mchc;
    }
    // Mean platelet volume
    if (meanPlateletVolume < 0) {
        box.min_edges[5] = meanPlateletVolume;
    } else {
        box.max_edges[5] = meanPlateletVolume;
    }

    // Leukocytes
    if (leukocytes < 0) {
        box.min_edges[6] = leukocytes;
    } else {
        box.max_edges[6] = leukocytes;
    }

    // Basophils
    if (basophils < 0) {
        box.min_edges[7] = basophils;
    } else {
        box.max_edges[7] = basophils;
    }

    // Eosinophils
    if (eosinophils < 0) {
        box.min_edges[8] = eosinophils;
    } else {
        box.max_edges[8] = eosinophils;
    }

    // Monocytes
    if (monocytes < 0) {
        box.min_edges[9] = monocytes;
    } else {
        box.max_edges[9] = monocytes;
    }

    // Proteina C reativa mg/dL
    if (crp < 0) {
        box.min_edges[10] = crp;
    } else {
        box.max_edges[10] = crp;
    }

    
    return box; // Devolvemos la caja con sus tamaños ajustados.
}



PatientData csvColumnsData(const string &line, int n)
{
    PatientData patient;
    stringstream ss(line);
    string token;

    int count = 0;
    while (getline(ss, token, ',') && count < n)
    {
        patient.characteristics.push_back(token); // Almacena en el primer vector
        count++;
    }

    return patient;
}

void csvDataPatient(const string &line2, int n, PatientData &patient,RStarTree<PatientData, 3, 10, 20> &rstarTree)
{
    stringstream ss2(line2);
    string token;
    string token2;
    patient.dataSubset.clear();
    int count = 0;
    while (getline(ss2, token2, ',') && count < n)
    {
        patient.dataSubset.push_back(token2); // Almacena en el segundo vector
        count++;
    }
    
    RStarBoundingBox<3> box=createBox3D(stoi(patient.dataSubset[0]),stoi(patient.dataSubset[1]),
    stoi(patient.dataSubset[2]),stoi(patient.dataSubset[3]),stoi(patient.dataSubset[4]),
    stoi(patient.dataSubset[5]),stoi(patient.dataSubset[6]),stoi(patient.dataSubset[7]),
    stoi(patient.dataSubset[8]),stoi(patient.dataSubset[9]),stoi(patient.dataSubset[10]));
    
    rstarTree.insert(patient, box);


}

int main()
{
    ifstream file0("./importantColumns.csv");
    ifstream file1("./dataColumn.csv");
    ifstream file2("./covid_DB.csv");
    string line0, line1, line2;

    RStarTree<PatientData, 3, 10, 20> rstarTree;


    getline(file1, line1);
    PatientData patient = csvColumnsData(line1,3);

    while (getline(file2, line2))
    {
        csvDataPatient(line2,3,patient, rstarTree);
    }
    
    // cout << "p" << endl;
    // cout << patient << endl;

    //rstarTree.print_tree(rstarTree.get_root(),0);



    return 0;
}
