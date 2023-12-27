#include "rstartree.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

struct Paciente
{
    double a, b, c, d, e, f, g, h, i, j, k;
};

std::ostream &operator<<(std::ostream &out, const Paciente &point)
{
    out << "(" << point.a << ", " << point.b << ", " << point.c
        << "(" << point.d << ", " << point.e << ", " << point.f
        << "(" << point.g << ", " << point.h << ", " << point.i
        << "(" << point.j << ", " << point.k << ")";
    return out;
}

// Función para crear una caja tridimensional
RStarBoundingBox<3> createBox3D(int a, int b, int c, int w, int h, int d)
{
    RStarBoundingBox<3> box;
    box.min_edges[0] = a;
    box.min_edges[1] = b;
    box.min_edges[2] = c;
    box.max_edges[0] = a + w;
    box.max_edges[1] = b + h;
    box.max_edges[2] = c + d;
    return box;
}

// Función para parsear una línea del archivo CSV b obtener un punto 3D
Paciente leerCSVLine(const string &line)
{
    Paciente caractPaciente;
    stringstream ss(line);
    string token;
    getline(ss, token, ',');
    caractPaciente.a = stod(token);
    getline(ss, token, ',');
    caractPaciente.b = stod(token);
    getline(ss, token, ',');
    caractPaciente.c = stod(token);
    getline(ss, token, ',');
    caractPaciente.d = stod(token);
    getline(ss, token, ',');
    caractPaciente.e = stod(token);
    getline(ss, token, ',');
    caractPaciente.f = stod(token);
    getline(ss, token, ',');
    caractPaciente.g = stod(token);
    getline(ss, token, ',');
    caractPaciente.h = stod(token);
    getline(ss, token, ',');
    caractPaciente.i = stod(token);
    getline(ss, token, ',');
    caractPaciente.j = stod(token);
    getline(ss, token, ',');
    caractPaciente.k = stod(token);
    return caractPaciente;
}

int main()
{
    // Crear un árbol R* para puntos 3D
    RStarTree<Paciente, 3, 10, 20> rstarTree; // Dimensiones: 3, Min Child: 10, Max Child: 20

    ifstream file("./Files/covid_DB_datos_importantes_completos_double.csv");
    string line;
    getline(file, line);

    while (getline(file, line))
    {
        // Parsear la línea b obtener un punto 3D
        Paciente caracteristicaPaciente = leerCSVLine(line);

        // Crear la caja tridimensional alrededor del punto
        // Solo sumamos 1
        RStarBoundingBox<3> box = createBox3D(caracteristicaPaciente.a, caracteristicaPaciente.b, caracteristicaPaciente.c, 1, 1, 1);

        rstarTree.insert(caracteristicaPaciente, box);
    }

    rstarTree.print_tree((rstarTree.get_root()), 0);

    // Eliminacion de datos

    auto box2 = createBox3D(1, 15, 0, 1, 1, 1);
    rstarTree.delete_objects_in_area(box2);
    // rstarTree.print_tree((rstarTree.get_root()),0);

    // auto box3 = createBox3D(1,15,0,1,3,2);
    // rstarTree.delete_objects_in_area(box3);
    rstarTree.print_tree((rstarTree.get_root()), 0);

    auto areafind = createBox3D(1, 15, 0, 1, 3, 2);
    auto structure_res = rstarTree.find_objects_in_area(areafind);
    for (int i = 0; i < structure_res.size(); i++)
    {
        cout << "$$$$$$$$" << endl;
        cout << structure_res[i].get_value() << endl;
    }

    return 0;
}
