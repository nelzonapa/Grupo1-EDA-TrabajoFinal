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
    vector<int> numberCharacteristica;
};

ostream &operator<<(ostream &out, const PatientData &paciente)
{
    for (int i = 0; i < paciente.characteristics.size(); i++)
    {
        out << "|" << paciente.characteristics[i] << "|\t";
        // out <<"(" << paciente.numberCharacteristica[i] <<")";
    }
    out << "\n";

    for (int i = 0; i < paciente.dataSubset.size(); i++)
    {
        out << "" << paciente.dataSubset[i] << "\t";
    }
    out << "\n";

    return out;
}

RStarBoundingBox<3> createBox3D(double testResult, double ageQuantile, double hemoglobin, 
double platelets, double meanPlateletVolume, double mchc, double leukocytes, 
double basophils, double eosinophils, double monocytes, double crp)
{
    RStarBoundingBox<3> box;  // Creamos la caja con tres tamaños diferentes

    // Estos valores representan los tamaños mínimos de la caja en cada dimensión
    box.min_edges[0] = testResult;
    box.min_edges[1] = ageQuantile;
    box.min_edges[2] = hemoglobin;
    
    // Al principio, los tamaños máximos son iguales a los tamaños mínimos,
    // así que la caja tiene el mismo tamaño en todas las dimensiones por ahora
    box.max_edges[0] = testResult;
    box.max_edges[1] = ageQuantile;
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



PatientData csvImportantData(const string &line0, int n, const string &line1)
{
    PatientData patient;
    stringstream ss1(line0);
    string token1;

    stringstream ss2(line1);
    string token2;

    int cont=0;
    getline(ss1, token1, ',');
    while (getline(ss2, token2, ','))
    {
        if (token2==token1)
        {
            patient.numberCharacteristica.push_back(cont); 
            getline(ss1, token1, ',');
            patient.characteristics.push_back(token1);
        }
        cont++;
    }

    return patient;
}


void csvDataPatient(const string &line2, int n, PatientData &patient,RStarTree<PatientData, 3, 10, 20> &rstarTree)
{
    stringstream ss2(line2);
    string token2;

    //avoid repeat data
    patient.dataSubset.clear();

    int i = 0;
    int cont = 0;
    while (getline(ss2, token2, ','))
    {
        if ((patient.numberCharacteristica[cont])==i)
        {
            if (token2=="\"positive\"" )
            {
                patient.dataSubset.push_back("1");
                cont++;
            }
            else if (token2=="\"negative\"")
            {
                patient.dataSubset.push_back("0");
                cont++;
            }
            else if(token2 =="\"\"")
            {
                patient.dataSubset.push_back("0");
                cont++;
            }
            else if(token2 =="\"detected\"")
            {
                patient.dataSubset.push_back("1");
                cont++;
            }
            else if(token2 =="\"not_detected\"")
            {
                patient.dataSubset.push_back("0");
                cont++;
            }
            else
            {
                token2=token2.substr(1, token2.size() - 2);
                patient.dataSubset.push_back(token2);
                cont++;
            }
            
        }
        i++;
    }


}

void menu2()
{   

    RStarTree<PatientData, 3, 10, 20> rstarTree;

    ifstream file2("./Files/covid_DB.csv");
    string line0, line1, line2;


    //cout << patient << endl;
    while (getline(file2, line2))
    {
        cout<<"$$$$$$$$$$$$$$$$"<<endl;
        ifstream file0("./Files/importantColumns.csv");
        ifstream file1("./Files/dataColumn.csv");
        getline(file0,line0);
        getline(file1, line1);
        PatientData patient = csvImportantData(line0,11, line1);
        file0.close();
        file1.close();


        csvDataPatient(line2,11,patient, rstarTree);
        cout<<patient<<endl;
    
        RStarBoundingBox<3> box=createBox3D(stod(patient.dataSubset[0]),stod(patient.dataSubset[1]),
        stod(patient.dataSubset[2]),stod(patient.dataSubset[3]),stod(patient.dataSubset[4]),
        stod(patient.dataSubset[5]),stod(patient.dataSubset[6]),stod(patient.dataSubset[7]),
        stod(patient.dataSubset[8]),stod(patient.dataSubset[9]),stod(patient.dataSubset[10]));
        // cout << "p2" << endl;

        // cout<<patient<<endl;
        rstarTree.insert(patient, box);

        
        patient.dataSubset.clear();
        patient.characteristics.clear();
        patient.numberCharacteristica.clear();
        box.reset();
        // cout << "p3" << endl;
    }
    file2.close();
    rstarTree.print_tree(rstarTree.get_root(),0);
}

int main(){
    menu2();
    return 0;
}
