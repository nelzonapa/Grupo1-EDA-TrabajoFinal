#pragma once
#include <iostream>
#include "boundingbox.h"
#include <cstddef>
#include <fstream>
#include <queue>
#include <unordered_set>
#include <vector>
#include "visualizer.h"
// using BoundingBox = RStarBoundingBox<2>;

using namespace std;

/*
T, que representa el tipo de datos que se almacenará en el árbol.
dimensions, que representa el número de dimensiones para la ubicación espacial de los datos.
min_child_items y max_child_items, que son parámetros para determinar cuántos elementos mínimos y máximos pueden estar en cada nodo del árbol.
*/

template <typename LeafType, size_t dimensions,
          size_t min_child_items, size_t max_child_items>

class RStarTree
{
    using BoundingBox = RStarBoundingBox<dimensions>;

private:
    /*
    TreePart: Representa una parte genérica del árbol,
    tiene un atributo BoundingBox llamado box. Esta estructura es
    una especie de "molde" común para nodos y hojas.
    */
    struct TreePart
    {
        BoundingBox box;
    };

    /*
    Node: Es una estructura que hereda de TreePart y
    representa los nodos internos del árbol. Contiene
    un vector de punteros a TreePart llamado items que
    almacenará las referencias a las partes del árbol
    (nodos u hojas) y un indicador hasleaves que informa
    si el nodo contiene hojas.
    */
    struct Node : public TreePart
    {
        vector<TreePart *> items;
        int hasleaves{false};
    };

    /*
    Leaf: Es una estructura que hereda de TreePart y
    representa las hojas del árbol. Contiene un atributo
    LeafType llamado value, que almacena el valor específico
     del tipo de hoja que se define en el árbol.
    */
    struct Leaf : public TreePart
    {
        LeafType value;
    };

    /*
    SplitParameters: Es una estructura que almacena parámetros
    relacionados con la división de nodos en el árbol. Tiene tres
    atributos: index (índice), axis (eje) y type (tipo de eje),
    utilizados durante las operaciones de división de nodos para
    determinar cómo se realizará la división.
    */
    struct SplitParameters
    {
        int index{-1};
        int axis{-1};
        axis_type type{axis_type::lower};
    };

public:
    class LeafWithConstBox
    {
    public:
        /*
        LeafWithConstBox se utiliza para representar una hoja del árbol
        con un área delimitadora constante. Se utiliza para devolver
        instancias de hojas desde el método de búsqueda del árbol.
        Esta clase proporciona métodos para acceder a los campos de la
        hoja de una manera controlada.

        LeafWithConstBox tiene métodos públicos como get_box() y get_value()
        para obtener la referencia constante a la caja delimitadora (BoundingBox)
        y al valor almacenado en la hoja (LeafType), respectivamente.
        */
        LeafWithConstBox(Leaf *leaf_) : leaf(leaf_) {}

        const BoundingBox &get_box() const { return leaf->box; }
        const BoundingBox &get_box() { return leaf->box; }
        const LeafType &get_value() const { return leaf->value; }

        LeafType &get_value() { return leaf->value; }
        bool operator<(const LeafWithConstBox &rhs) const
        {
            if (get_value() == rhs.get_value())
            {
                return get_box() < rhs.get_box();
            }
            return get_value() < rhs.get_value();
        }
        bool operator==(const LeafWithConstBox &rhs) const
        {
            return get_value() == rhs.get_value() && get_box() == rhs.get_box();
        }
        bool operator!=(const LeafWithConstBox &rhs) const
        {
            return !operator==(rhs);
        }

    public:
        Leaf *leaf{nullptr};
    };

public:
    RStarTree()
    {
        if (dimensions <= 0 || max_child_items < min_child_items)
        {
            throw invalid_argument("");
        }
    }
    ~RStarTree() { delete_tree(tree_root); }

    /*
    La función insert() permite insertar una hoja en el
    árbol. Toma como argumentos un valor de hoja (LeafType)
     y un área delimitadora (BoundingBox).
     Dentro de esta función:

    * Se incrementa el tamaño (size_) del árbol.
    * Se crea una nueva hoja Leaf y se le asigna el valor
        proporcionado (leaf) y la caja delimitadora (box).
    * Si el árbol está vacío (!tree_root), se crea un nuevo
     nodo raíz y se inserta la hoja en él.
    * En caso contrario, se invoca a la función
    choose_leaf_and_insert() para encontrar el nodo
    hoja adecuado y realizar la inserción.
    * used_deeps.clear() se usa para limpiar un contenedor
    (posiblemente un conjunto o un vector) llamado used_deeps.

    */
    void insert(const LeafType leaf, const BoundingBox &box)
    {
        size_++;
        Leaf *new_leaf = new Leaf;
        new_leaf->value = leaf;
        new_leaf->box = box;
        if (!tree_root)
        {
            tree_root = new Node();
            tree_root->hasleaves = true;
            tree_root->items.reserve(min_child_items);
            tree_root->items.push_back(new_leaf);
        }
        else
        {
            choose_leaf_and_insert(new_leaf, tree_root);
        }
        used_deeps.clear();
    }

    /*
    find_objects_in_area() es un método que busca objetos
    dentro de un área específica. Toma como argumento un cuadro
    delimitador (BoundingBox). Dentro de esta función:

    * Se crea un vector de LeafWithConstBox llamado leafs.
    * Se llama a la función find_leaf() para buscar las hojas
    que se encuentran dentro del área especificada. Estas hojas
    se almacenan en el vector leafs.
    * Retorna el vector leafs, que contiene las hojas encontradas
    dentro del área.
    */
    vector<LeafWithConstBox> find_objects_in_area(const BoundingBox &box)
    {
        vector<LeafWithConstBox> leafs;
        find_leaf(box, leafs, tree_root);
        return leafs;
    }

    /*
    delete_objects_in_area() es un método que elimina objetos
    dentro de un área específica. Toma como argumento un cuadro
    delimitador (BoundingBox).
    Dentro de esta función:

        * función delete_leafs() para eliminar las hojas que se
        encuentran dentro del área especificada del árbol.
    */
    void delete_objects_in_area(const BoundingBox &box)
    {
        delete_leafs(box, tree_root);
    }

private:
    void write_node(Node *node, fstream &file)
    {
        size_t _size = node->items.size();
        file.write(reinterpret_cast<const char *>(&_size), sizeof(_size));
        file.write(reinterpret_cast<const char *>(&(node->hasleaves)),
                   sizeof(node->hasleaves));
        for (size_t axis = 0; axis < dimensions; axis++)
        {
            file.write(reinterpret_cast<const char *>(&(node->box.max_edges[axis])),
                       sizeof(node->box.max_edges[axis]));
            file.write(reinterpret_cast<const char *>(&(node->box.min_edges[axis])),
                       sizeof(node->box.min_edges[axis]));
        }
    }

    void write_leaf(Leaf *leaf, fstream &file)
    {
        for (size_t axis = 0; axis < dimensions; axis++)
        {
            file.write(reinterpret_cast<char *>(&(leaf->box.max_edges[axis])),
                       sizeof(leaf->box.max_edges[axis]));
            file.write(reinterpret_cast<char *>(&(leaf->box.min_edges[axis])),
                       sizeof(leaf->box.min_edges[axis]));
        }
        file.write(reinterpret_cast<char *>(&(leaf->value)), sizeof(LeafType));
        size_--;
    }

    Node *read_node(fstream &file)
    {
        size_t _size;
        file.read(reinterpret_cast<char *>(&_size), sizeof(_size));
        Node *new_node = new Node;
        new_node->items.resize(_size);
        file.read(reinterpret_cast<char *>(&(new_node->hasleaves)),
                  sizeof(new_node->hasleaves));
        for (size_t axis = 0; axis < dimensions; axis++)
        {
            file.read(reinterpret_cast<char *>(&(new_node->box.max_edges[axis])),
                      sizeof(new_node->box.max_edges[axis]));
            file.read(reinterpret_cast<char *>(&(new_node->box.min_edges[axis])),
                      sizeof(new_node->box.min_edges[axis]));
        }
        return new_node;
    }

    Leaf *read_leaf(fstream &file)
    {
        Leaf *new_leaf = new Leaf;
        for (size_t axis = 0; axis < dimensions; axis++)
        {
            file.read(reinterpret_cast<char *>(&(new_leaf->box.max_edges[axis])),
                      sizeof(new_leaf->box.max_edges[axis]));
            file.read(reinterpret_cast<char *>(&(new_leaf->box.min_edges[axis])),
                      sizeof(new_leaf->box.min_edges[axis]));
        }
        file.read(reinterpret_cast<char *>(&(new_leaf->value)), sizeof(LeafType));
        size_++;
        return new_leaf;
    }

    /*
    La función `find_leaf` es crucial en la búsqueda de hojas
    dentro del árbol R-Star que están contenidas dentro de un área
    determinada (`BoundingBox`). Aquí está su desglose:

    - `void find_leaf(const BoundingBox &box,
    vector<LeafWithConstBox> &leafs, Node *node)`:
    Esta función toma tres argumentos:
        - `box`: El área de búsqueda representada por
        un cuadro delimitador.
        - `leafs`: Un vector que almacenará las hojas
        encontradas dentro del área.
        - `node`: El nodo actual en el que se está buscando.

    La función opera de la siguiente manera:

    - Verifica si el nodo actual contiene hojas (`node->hasleaves`).
    Si es así, itera a través de las hojas presentes en ese nodo.
        - Para cada hoja, se convierte el elemento del nodo a un puntero `Leaf`.
        - Comprueba si la caja delimitadora de esa hoja (`temp_leaf->box`)
        se intersecta con el área de búsqueda (`box`).
        - Si hay una intersección, agrega esa hoja al vector `leafs`.

    - Si el nodo actual no contiene hojas (`node->hasleaves` es falso),
    significa que contiene nodos secundarios.
        - Itera a través de los nodos secundarios presentes en el
        nodo actual.
        - Para cada nodo secundario, se convierte el elemento del nodo
        a un puntero `Node`.
        - Comprueba si la caja delimitadora de ese nodo secundario
        (`temp_node->box`) se intersecta con el área de búsqueda (`box`).
        - Si hay una intersección, se llama recursivamente a
        `find_leaf()` con ese nodo secundario como nuevo nodo actual.

    Este proceso continúa hasta que se recorren todos los nodos del árbol
    y se encuentran todas las hojas que se superponen con el área de búsqueda.
    Las hojas encontradas se almacenan en el vector `leafs`, que luego se
    devuelve a la función que la llamó.
    */
    void find_leaf(const BoundingBox &box, vector<LeafWithConstBox> &leafs,
                   Node *node)
    {
        if (node->hasleaves)
        {
            for (size_t i = 0; i < node->items.size(); i++)
            {
                Leaf *temp_leaf = static_cast<Leaf *>(node->items[i]);
                if (box.is_intersected((temp_leaf->box)))
                {
                    leafs.push_back({temp_leaf});
                }
            }
        }
        else
        {
            for (size_t i = 0; i < node->items.size(); i++)
            {
                Node *temp_node = static_cast<Node *>(node->items[i]);
                if (box.is_intersected((temp_node->box)))
                {
                    find_leaf(box, leafs, temp_node);
                }
            }
        }
    }

    /*
    La función `delete_leafs` es esencial para la eliminación
    de elementos dentro de un área específica del árbol R-Star.
    Aquí está la explicación línea por línea:

    1. `void delete_leafs(const BoundingBox &box, Node *node)`:
    Esta función recibe dos parámetros: un cuadro delimitador
    (`BoundingBox`) que define el área de eliminación y un
    puntero a un nodo del árbol R-Star.

    2. `if (node->hasleaves) { / ... / } else { / ... / }`:
    Comprueba si el nodo actual contiene hojas (`hasleaves`
    es verdadero). Si es así, se ejecuta el primer bloque de
    código. De lo contrario, se ejecuta el segundo bloque.

    3. En el bloque `if (node->hasleaves)`: Si el nodo contiene
    hojas, itera a través de las hojas y verifica si cada una
    se intersecta con el cuadro delimitador pasado como argumento.

        - Si una hoja se intersecta con el cuadro delimitador,
        se intercambia con la última hoja (`swap`) y luego se
        elimina (`delete`) esa última hoja. Posteriormente, se
        quita del vector de hojas del nodo (`pop_back`) y se
        reduce el índice `i` para volver a comprobar la nueva
        hoja en la posición actual.

        - Luego, se resetea (`reset`) el cuadro delimitador
        del nodo actual (`node->box`).

    4. En el bloque `else`, si el nodo no contiene hojas, itera
    a través de los elementos (niños) del nodo y verifica si se
    intersectan con el cuadro delimitador pasado como argumento.

        - Si un niño se intersecta con el cuadro delimitador, se
        llama recursivamente a `delete_leafs` para ese niño.

    5. Al final, independientemente de si el nodo tiene hojas o
    no, se realiza una iteración adicional para ajustar (`stretch`)
    el cuadro delimitador del nodo actual basado en sus elementos
    (hojas o nodos hijos).

    Esta función es crucial para eliminar todos los elementos
    dentro de un área específica dentro del árbol R-Star, manejando
    tanto hojas como nodos internos dependiendo de la estructura
    del árbol en cada nivel.
    */
    void delete_leafs(const BoundingBox &box, Node *node)
    {
        if (node->hasleaves)
        { // If the children of an area are leaves, then all
          // children are tested.
            for (size_t i = 0; i < node->items.size(); i++)
            {
                if (box.is_intersected((node->items[i]->box)))
                {
                    swap(node->items[i],
                         node->items.back());  // changing from the last one
                    delete node->items.back(); // delete the last one
                    node->items.pop_back();
                    i--;
                }
            }
            node->box.reset();
        }
        else
        {
            for (size_t i = 0; i < node->items.size(); i++)
            {
                if (box.is_intersected((node->items[i]->box)))
                {
                    delete_leafs(box,
                                 static_cast<Node *>(
                                     node->items[i])); // we call the method from those
                                                       // eedges whose regions intersect
                }
            }
        }
        for (size_t i = 0; i < node->items.size(); i++)
        {
            node->box.stretch(node->items[i]->box);
        }
    }

    /*

    Parámetros:

    * Leaf *leaf: La hoja que se desea insertar.
    * Node *node: El nodo actual en el que se
    está considerando la inserción.
    * int deep = 0: Un parámetro opcional que lleva
    un registro de la profundidad en el árbol.

    Acciones:

    * node->box.stretch(leaf->box): Se expande la caja del
    nodo actual (node) para abarcar también la caja de la
    hoja que se va a insertar (leaf->box). Esto asegura que
    el nodo contenga ambos límites.

    * if (node->hasleaves) { ... } else { ... }: Se verifica
    si el nodo actual tiene hojas como hijos. Si es así, se
    agrega directamente la hoja al conjunto de elementos
    (node->items). De lo contrario, se continúa buscando un
    nodo hoja adecuado para la inserción.

    * Node *new_node = choose_leaf_and_insert(leaf,
    choose_subtree(node, leaf->box), deep + 1):
    Si el nodo actual no tiene hojas como hijos, se invoca
    recursivamente la función choose_leaf_and_insert. Esta
    llamada encuentra el nodo hoja apropiado (choose_subtree())
    para insertar la hoja y realiza la inserción en ese nodo.
    El incremento de la profundidad (deep + 1) ayuda a rastrear
    la profundidad del árbol.

    * if (!new_node) { ... }: Si la función choose_leaf_and_insert
     no devuelve un nodo (lo que indica que no hubo una división),
     se interrumpe la recursión y no se hace nada más con respecto
     a la inserción actual.

    * if (node->items.size() > max_child_items) { ... }: Finalmente,
    se verifica si la cantidad de elementos en el nodo actual supera
    el límite establecido (max_child_items). Si es así, se activa un
    procedimiento de tratamiento de desbordamiento (overflow_treatment)
    que dividirá el nodo en dos.

    Resultado:

    Se devuelve un puntero Node* que indica si ha ocurrido un
    desbordamiento (overflow_treatment) y se ha dividido el
    nodo actual. Si no hay desbordamiento, se devuelve nullptr.

    */
    Node *choose_leaf_and_insert(Leaf *leaf, Node *node, int deep = 0)
    {
        node->box.stretch(leaf->box);
        if (node->hasleaves)
        { // If the children of the node are leaves, add a
          // leaf to the array
            node->items.push_back(leaf);
        }
        else
        {
            Node *new_node = choose_leaf_and_insert(
                leaf, choose_subtree(node, leaf->box), deep + 1);
            if (!new_node)
            { // choose_leaf_and_insert will return the location, it
              // will be inserted into the children's array
                return nullptr;
            }
            node->items.push_back(new_node);
        }
        if (node->items.size() > max_child_items)
        {
            return overflow_treatment(
                node, deep); // If the number of children is greater than
                             // max_child_items, the node must be divided.
        }
        return nullptr;
    }

    /*
     `choose_node_and_insert`, tiene un papel crucial
     en la inserción de nodos dentro del árbol R-Star.
     Aquí está su funcionamiento detallado:

    - `Node *choose_node_and_insert(
        Node *node, Node *parent_node, int required_deep, int deep = 0)`:
        Esta función toma como argumentos el nodo a insertar (`node`),
         el nodo padre (`parent_node`), la profundidad requerida
         (`required_deep`), y opcionalmente, la profundidad actual (`deep`).

    Dentro de esta función:

    - Se ajusta la caja delimitadora del `parent_node` para cubrir
    la caja del `node` que se va a insertar (`parent_node->box.stretch(node->box)`).

    - Si la profundidad actual (`deep`) es igual a la profundidad
    requerida (`required_deep`), se agrega el nodo al vector de
     ítems del `parent_node`. Esto se hace para establecer la
      ubicación del nodo a una profundidad predeterminada,
      manteniendo el equilibrio en el árbol.

    - Si no se ha alcanzado la profundidad requerida, se continúa
    descendiendo en el árbol llamando recursivamente a
    `choose_node_and_insert` con un incremento en la profundidad
    (`deep + 1`). El nuevo nodo resultante de la recursión se agrega
     al vector de ítems del `parent_node`.

    - Después de la inserción, si el tamaño del vector de ítems
    del `parent_node` supera el límite máximo (`max_child_items`),
    se llama a la función `overflow_treatment()` para tratar el
    desbordamiento y realizar las acciones correspondientes para
    mantener la estructura del árbol equilibrada.

    En resumen, esta función decide dónde insertar un nodo dentro
    del árbol R-Star, asegurando que se mantenga el equilibrio y
    la organización de los nodos en las profundidades requeridas.

    */
    Node *choose_node_and_insert(Node *node, Node *parent_node, int required_deep,
                                 int deep = 0)
    {
        parent_node->box.stretch(node->box);
        if (deep ==
            required_deep)
        { // Similar to choose_leaf_and_insert, only for nodes
            parent_node->items.push_back(
                node); // The location is set at a predetermined depth so that the
                       // tree remains perfectly balanced.
        }
        else
        {
            Node *new_node =
                choose_node_and_insert(node, choose_subtree(parent_node, node->box),
                                       required_deep, deep + 1);
            if (!new_node)
                return nullptr;
            parent_node->items.push_back(new_node);
        }
        if (parent_node->items.size() > max_child_items)
        {
            return overflow_treatment(parent_node, deep);
        }
        return nullptr;
    }

    /*
    `choose_subtree` es esencial para determinar el nodo adecuado
    dentro del árbol R-Star donde se debe insertar un nuevo elemento.
    Aquí está la explicación detallada:

    - `Node *choose_subtree(Node *node, const BoundingBox &box)`:
    Esta función selecciona el subárbol apropiado para insertar
    un nuevo elemento, dado un nodo y una caja delimitadora (`BoundingBox`).

    En detalle:

    - Si los nodos hijos son terminales (es decir, hojas),
    la función busca el nodo con el menor solapamiento con
    la caja pasada como argumento.
    - Itera sobre los nodos hijos del nodo proporcionado (`node`),
    calculando el aumento del solapamiento con la caja
    (`box.area() - box.overlap(temp->box)`).
    - Encuentra el nodo con el menor aumento de solapamiento y
    lo guarda en `overlap_preferable_nodes`.
    - Si hay un solo nodo con el mínimo solapamiento, se devuelve ese nodo.

    - Si los nodos hijos no son terminales, se guardan
    todos en `overlap_preferable_nodes`.

    - Luego, entre los nodos preferibles obtenidos,
    se busca aquel con el menor aumento de área si
    hay varios nodos con el mismo solapamiento mínimo.
    - Se estira temporalmente la caja con cada nodo preferible
    y se calcula el aumento de área.
    - Se encuentra el nodo con el menor aumento de área y
    se guarda en `area_preferable_nodes`.
    - Si hay un solo nodo con el mínimo aumento de área,
    se devuelve ese nodo.

    - Si aún hay varios nodos en `area_preferable_nodes`,
    se busca entre ellos aquel con el área mínima y se devuelve
    como el subárbol seleccionado para insertar el nuevo elemento.

    - Después de encontrar los nodos preferidos basados en el
    solapamiento de las cajas y, si es necesario, en el aumento
    de área, se realiza una serie de comparaciones para seleccionar
    el nodo más adecuado como subárbol para la inserción:

    - Si hay un solo nodo con el mínimo solapamiento o aumento de
    área, se devuelve ese nodo como el subárbol seleccionado para
    insertar el nuevo elemento.

    - Sin embargo, si aún hay varios nodos en `area_preferable_nodes`,
     se busca entre ellos el que tenga el área mínima. Esta sección
     busca el nodo entre los restantes nodos preferidos con el área
     más pequeña posible para mantener un criterio más estricto al
     elegir el subárbol de inserción.

    - La variable `min_area_node` se inicializa como `nullptr` y
    `min_area` con el valor máximo de `int`. Luego, se compara el
    área de cada nodo en `area_preferable_nodes` y se actualiza
    `min_area_node` si encuentra un nodo con un área menor.

    - Al final, se devuelve el nodo con el área más pequeña como el
    subárbol seleccionado para insertar el nuevo elemento.

    Esta función es fundamental para el proceso de inserción en el
    árbol R-Star, ya que ayuda a elegir el nodo más adecuado en
    función del solapamiento y el área para mantener la estructura
    del árbol de manera equilibrada y eficiente.
    */
    Node *choose_subtree(Node *node, const BoundingBox &box)
    {
        vector<TreePart *> overlap_preferable_nodes;
        if (static_cast<Node *>(node->items[0])
                ->hasleaves)
        { // If the child nodes are terminal nodes, the node
          // with the smallest overlap is searched for
            int min_overlap_enlargement(numeric_limits<int>::max());
            int overlap_enlargement(0);
            for (size_t i = 0; i < node->items.size(); i++)
            {
                TreePart *temp = (node->items[i]);
                overlap_enlargement = box.area() - box.overlap(temp->box);
                if (overlap_enlargement < min_overlap_enlargement)
                {
                    min_overlap_enlargement = overlap_enlargement;
                    overlap_preferable_nodes.resize(0);
                    overlap_preferable_nodes.push_back(temp);
                }
                else
                {
                    if (overlap_enlargement == min_overlap_enlargement)
                    {
                        overlap_preferable_nodes.push_back(temp);
                    }
                }
            }
            if (overlap_preferable_nodes.size() ==
                1)
            { // If there's only one node, it's a special node.
                return static_cast<Node *>(overlap_preferable_nodes.front());
            } // If not, then a space with the smallest possible increase in area is
              // searched for
        }
        else
        { // If the child nodes are not terminal, keep them in the array
            overlap_preferable_nodes.reserve(node->items.size());
            copy(node->items.begin(), node->items.end(),
                 back_inserter(overlap_preferable_nodes));
        }
        int min_area_enlargement =
            numeric_limits<int>::max(); // for both terminal and nonterminal
        int area_enlargement(0);        // subsequent steps are the same
        vector<TreePart *> area_preferable_nodes;
        for (size_t i = 0; i < overlap_preferable_nodes.size(); i++)
        {
            BoundingBox temp(box);
            temp.stretch(overlap_preferable_nodes[i]->box);
            area_enlargement = temp.area() - box.area();
            if (min_area_enlargement > area_enlargement)
            {
                min_area_enlargement = area_enlargement;
                area_preferable_nodes.resize(0);
                area_preferable_nodes.push_back(overlap_preferable_nodes[i]);
            }
            else
            {
                if (min_area_enlargement == area_enlargement)
                {
                    area_preferable_nodes.push_back(overlap_preferable_nodes[i]);
                }
            }
        }
        if (area_preferable_nodes.size() ==
            1)
        { // If there is only one minimum-increase-area node, it will be
          // returned
            return static_cast<Node *>(area_preferable_nodes.front());
        }
        TreePart *min_area_node{nullptr}; // Looking for a node among the remaining
                                          // ones with the smallest possible area
        int min_area(numeric_limits<int>::max());
        for (size_t i = 0; i < area_preferable_nodes.size(); i++)
        {
            if (min_area > area_preferable_nodes[i]->box.area())
            {
                min_area_node = area_preferable_nodes[i];
            }
        }
        return static_cast<Node *>(min_area_node);
    }

    /*
    Parámetros:

    * Node *node: El nodo que ha excedido la cantidad máxima de elementos.
    * int deep: La profundidad en la que se encuentra el nodo en el árbol.

    Acciones:

    * if (used_deeps.count(deep) == 0 && tree_root != node) { ... }:
    Se verifica si ya se ha realizado un tratamiento de desbordamiento
    en esta profundidad (deep) y si el nodo no es la raíz del árbol.
    Si estas condiciones se cumplen, se activa el método de reinserción
    forzada (forced_reinsert) y se devuelve nullptr. Esta es una técnica
    para equilibrar el árbol, permitiendo una única reinserción por
    profundidad y evitando que la raíz del árbol sea reinsertada.

    * Node *splitted_node = split(node);: Se invoca la función split
    para dividir el nodo en dos. Esta operación elimina los elementos
    innecesarios del nodo y devuelve un puntero al nuevo nodo creado
    tras la división.

    * if (node == tree_root) { ... }: Si el nodo desbordado es la
    raíz del árbol, se crea un nuevo nodo temporal (temp) que crece
    un nivel hacia arriba. Este nuevo nodo contendrá la raíz anterior
     y el nodo recién dividido como sus elementos. Se restablece la
     caja del nodo raíz y se estira para abarcar las cajas de sus dos hijos.

    * Se devuelve un puntero: Si el nodo desbordado no es la raíz
    del árbol, se devuelve el puntero al nodo dividido (splitted_node).
    Este puntero indica el lugar donde se deben insertar los hijos
    divididos en el array del padre del nodo desbordado.

    Resultado:

    * La función devuelve un puntero Node* que indica cómo se ha resuelto
     el desbordamiento: si se realizó la división y se necesita insertar
     los nodos resultantes en el padre o si se ejecutó una reestructuración
      específica en la raíz del árbol.
    */
    Node *overflow_treatment(Node *node, int deep)
    {
        if (used_deeps.count(deep) == 0 &&
            tree_root != node)
        { // The reinsertion method can be used only once
          // per depth and not for the root.
            forced_reinsert(node, deep);
            return nullptr;
        }
        Node *splitted_node =
            split(node); // deletes unnecessary children from the node and returns
                         // the location with them
        if (node == tree_root)
        { // If node is the root of a tree, it grows one
          // level upward
            Node *temp = new Node;
            temp->hasleaves = false;
            temp->items.reserve(min_child_items);
            temp->items.push_back(tree_root);
            temp->items.push_back(splitted_node);
            tree_root = temp;
            tree_root->box.reset();
            tree_root->box.stretch(temp->items[0]->box);
            tree_root->box.stretch(temp->items[1]->box);

            return nullptr;
        }
        return splitted_node; // Otherwise, the location is returned for the
                              // insertion of the children into the parent's
                              // array.
    }

    /*
    Claro, en la función `split`, ocurre la división de un nodo
    que ha excedido el límite de capacidad (`max_child_items`).
    Aquí está el análisis línea por línea:

    - `SplitParameters params = choose_split_axis_and_index(node);`:
    Esta línea determina el eje de división y el índice en el
    que se dividirá el nodo. `choose_split_axis_and_index`
    devuelve los parámetros más óptimos para dividir el nodo.

    - `sort(node->items.begin(), node->items.end(), [&params](auto lhs, auto rhs) {...});`:
    Los elementos del nodo se ordenan según el eje y el
    índice elegidos para la división. La función lambda
    define cómo se ordenarán los elementos basados en el
    valor del eje y el tipo de división.

    - `Node *new_Node = new Node;`: Se crea un nuevo nodo para
    almacenar los elementos que se separarán del nodo original.

    - `new_Node->items.reserve(max_child_items + 1
    - min_child_items - params.index);`:
    Se reserva espacio en el nuevo nodo para los elementos que serán movidos a este nodo.

    - `copy(node->items.begin() + min_child_items +
    params.index, node->items.end(), back_inserter(new_Node->items));`:
    Los elementos seleccionados para la división se copian al
    nuevo nodo, y luego se eliminan del nodo original
    usando `node->items.erase`.

    - Se actualizan las cajas del nodo original (`node->box`)
     y del nuevo nodo (`new_Node->box`) para reflejar los
     cambios de elementos en ambos nodos.

    - Finalmente, se devuelve el nuevo nodo creado, que contiene
    los elementos que se han separado del nodo original, y el
    nodo original ha sido modificado para reflejar
    la eliminación de estos elementos.

    Esta función es esencial para mantener el equilibrio y la
    capacidad adecuada de los nodos en el árbol R-Star al
    dividir un nodo cuando excede su capacidad máxima permitida.
    */
    Node *split(Node *node)
    {
        SplitParameters params = choose_split_axis_and_index(
            node); // The most optimal index and axis are selected
        sort(node->items.begin(), node->items.end(),
             [&params](auto lhs, auto rhs)
             {
                 return lhs->box.value_of_axis(params.axis, params.type) <
                        rhs->box.value_of_axis(params.axis, params.type);
             });
        Node *new_Node = new Node;
        new_Node->items.reserve(max_child_items + 1 - min_child_items -
                                params.index);
        new_Node->hasleaves = node->hasleaves;
        copy(node->items.begin() + min_child_items + params.index,
             node->items.end(), back_inserter(new_Node->items));
        node->items.erase(node->items.begin() + min_child_items + params.index,
                          node->items.end());
        new_Node->box.reset();
        node->box.reset();
        for (auto &w : node->items)
        {
            node->box.stretch(w->box);
        }
        for (auto &w : new_Node->items)
        {
            new_Node->box.stretch(w->box);
        }
        return new_Node;
    }

    /*
    La función `forced_reinsert` se encarga de reinsertar
    algunos de los hijos de un nodo dado dentro del árbol.
    Aquí está el análisis línea por línea:

    - `double p = 0.3;`: Esta línea define el porcentaje de
    hijos que se eliminarán del nodo actual para ser reinsertados en el árbol.

    - `int number = node->items.size() * p;`: Se calcula el
    número de elementos que se eliminarán del nodo para su
    reinyección, basado en el porcentaje definido.

    - `sort(node->items.begin(), node->items.end(),
    [&node](auto lhs, auto rhs) {...});`:
    Los elementos del nodo se ordenan según la distancia entre
    sus centros y el centro del nodo actual. Esta función lambda
    define cómo se ordenarán los elementos basados en la
    distancia de los centros de las cajas delimitadoras.

    - Se crea un vector `forced_reinserted_nodes` para
    almacenar los elementos seleccionados para la reinyección.

    - `copy(node->items.rbegin(), node->items.rbegin() +
     number, back_inserter(forced_reinserted_nodes));`:
     Los elementos seleccionados para reinyección se copian
     al vector `forced_reinserted_nodes` desde el final
     del vector `node->items`.

    - `node->items.erase(node->items.end() - number, node->items.end());`:
     Se eliminan los elementos seleccionados para reinyección del vector original del nodo.

    - Se actualiza la caja del nodo (`node->box`) para reflejar
     la eliminación de los elementos seleccionados.

    - Se agrega el nivel actual al conjunto `used_deeps` para
    evitar la reinyección en el mismo nivel.

    - Se recorren los elementos seleccionados para reinyección,
     y si son hojas, se vuelven a insertar usando `choose_leaf_and_insert`,
      y si son nodos, se insertan utilizando `choose_node_and_insert`
      hasta una profundidad especificada.

    Esta función es crucial para mantener la estructura balanceada
    del árbol, ya que permite redistribuir algunos elementos en el
    árbol cuando un nodo alcanza una capacidad máxima y necesita reducir su contenido.
    */
    void forced_reinsert(Node *node,
                         int deep)
    {                   // Some of the children of the given tree
                        // are reinserted into the tree
        double p = 0.3; // Percentage of children that will be deleted from the
                        // node location
        int number = node->items.size() * p;
        sort(node->items.begin(), node->items.end(),
             [&node](auto lhs, auto rhs)
             {
                 return lhs->box.dist_between_centers(node->box) <
                        rhs->box.dist_between_centers(node->box);
             });
        vector<TreePart *> forced_reinserted_nodes;
        forced_reinserted_nodes.reserve(number);
        copy(node->items.rbegin(), node->items.rbegin() + number,
             back_inserter(forced_reinserted_nodes));
        node->items.erase(node->items.end() - number, node->items.end());
        node->box.reset();
        used_deeps.insert(deep);
        for (TreePart *w : node->items)
        {
            node->box.stretch(w->box);
        }
        if (node->hasleaves) // If the children of the top are leaves, they are
                             // inserted again using the method
                             // choose_leaf_and_insert.
            for (TreePart *w : forced_reinserted_nodes)
            {
                choose_leaf_and_insert(static_cast<Leaf *>(w), tree_root, 0);
            }
        else
        { // If nodes - by the method choose_node_and_insert to a specified
          // depth
            for (TreePart *w : forced_reinserted_nodes)
            {
                choose_node_and_insert(static_cast<Node *>(w), tree_root, deep, 0);
            }
        }
    }

    /*
    La función `choose_split_axis_and_index` determina el eje
    y el índice óptimos para dividir un nodo en dos partes.
    Aquí tienes un análisis detallado:

    - `int min_margin(numeric_limits<int>::max());`:
    Inicializa una variable para rastrear el margen mínimo
    encontrado durante el proceso de división.

    - `int distribution_count(max_child_items - 2 * min_child_items + 2);`:
    Calcula el número total de distribuciones posibles para
     los elementos en los nodos hijos resultantes de la división.

    - `SplitParameters params;`: Crea una instancia de
    `SplitParameters` para almacenar los resultados de
    la división (índice, eje y tipo).

    - Se realiza un bucle a través de los ejes (dimensiones)
    posibles para la división.

    - Dentro de cada eje, se evalúan dos tipos de divisiones:
    en el límite más pequeño o en el límite más grande del eje.

    - Se ordenan los elementos del nodo según el valor del
    eje y tipo actuales utilizando una función lambda.

    - Luego, se itera sobre las posibles distribuciones
    dentro del rango permitido (`distribution_count`),
    evaluando cada división posible en dos conjuntos (b1 y b2)
     y calculando el margen total.

    - Se actualiza `min_margin` si se encuentra un margen menor
     en la distribución actual, y se actualizan los parámetros
     (`params.index`, `params.axis` y `params.type`) para reflejar
      la mejor división encontrada hasta ahora.

    - Finalmente, se devuelve la estructura `SplitParameters` con
     los valores óptimos para dividir el nodo.

    En resumen, esta función determina la mejor manera de dividir
     un nodo en dos partes en un eje específico y un punto específico
      a lo largo de ese eje, buscando minimizar el margen entre
      las cajas delimitadoras resultantes. Esto es esencial para
       mantener el equilibrio y la eficiencia del R*-Tree durante
       las inserciones y las operaciones de división.
    */
    SplitParameters choose_split_axis_and_index(Node *node)
    {
        int min_margin(numeric_limits<int>::max());
        int distribution_count(max_child_items - 2 * min_child_items + 2);
        SplitParameters params;
        BoundingBox b1, b2;
        for (int axis(0); axis < dimensions;
             axis++)
        { // the axis of the best allocation is selected
            for (int i(0); i < 2; i++)
            { // on the larger or on the smaller border
                axis_type type;
                if (i == 0)
                {
                    type = axis_type::lower;
                }
                else
                {
                    type = axis_type::upper;
                }
                sort(node->items.begin(), node->items.end(),
                     [&axis, &type](auto &lhs, auto &rhs)
                     {
                         return lhs->box.value_of_axis(axis, type) <
                                rhs->box.value_of_axis(axis, type);
                     });
                for (int k(0); k < distribution_count; k++)
                {
                    int area(0);
                    b1.reset();
                    b2.reset();
                    for (int i = 0; i < min_child_items + k; i++)
                    {
                        b1.stretch(node->items[i]->box);
                    }
                    for (int i = min_child_items + k; i < max_child_items + 1; i++)
                    {
                        b2.stretch(node->items[i]->box);
                    }
                    int margin = b1.margin() + b2.margin();
                    if (margin < min_margin)
                    {
                        min_margin = margin;
                        params.index = k;
                        params.axis = axis;
                        params.type = type;
                    }
                }
            }
        }
        return params;
    }

    /*
    La función `delete_tree` se encarga de liberar la
    memoria utilizada por un árbol R*-Tree, eliminando
    recursivamente todos los nodos y hojas del árbol.

    - `void delete_tree(Node *node)`: Esta función toma
    un nodo como argumento para comenzar la eliminación
    del árbol desde ese nodo hacia abajo.

    - Se comprueba si el nodo actual contiene hojas (leaf nodes).
    Si es así, se procede a eliminar cada hoja presente en el nodo.

    - Si el nodo no contiene hojas, significa que son nodos internos.
    Se itera sobre los elementos del nodo, y si son nodos internos,
    se llama recursivamente a `delete_tree` para eliminar los nodos
    descendientes.

    - Una vez que todos los descendientes han sido eliminados, se
    elimina el nodo actual (`node`) con `delete node;`, liberando
    así la memoria que ocupaba en el programa.

    Esta función asegura que se libere de manera adecuada y completa
    toda la memoria asignada para los nodos y hojas del árbol,
    evitando fugas de memoria. Es esencial para evitar problemas
    de gestión de memoria cuando el árbol ya no se necesita
    y se debe destruir.
    */
    void delete_tree(Node *node)
    {
        if (node->hasleaves)
        {
            for (size_t i = 0; i < node->items.size(); i++)
            {
                delete node->items[i];
            }
        }
        else
        {
            for (size_t i = 0; i < node->items.size(); i++)
            {
                delete_tree(static_cast<Node *>(node->items[i]));
            }
        }
        delete node;
    }

public:
    Node *get_root()
    {
        return tree_root;
    }
    void print_tree(Node *node, int depth)
    {
        if (node->hasleaves)
        {
            for (size_t i = 0; i < node->items.size(); i++)
            {
                Leaf *temp_leaf = static_cast<Leaf *>(node->items[i]);
                cout << string(4 * depth, ' ') << "Leaf: Value = " << temp_leaf->value << ", Box = [";
                for (size_t j = 0; j < dimensions; j++)
                {
                    cout << "(" << temp_leaf->box.min_edges[j] << ", " << temp_leaf->box.max_edges[j] << ") ";
                }
                cout << "]" << endl;
            }
        }
        else
        {
            cout << string(4 * depth, ' ') << "Node:" << endl;
            for (size_t i = 0; i < node->items.size(); i++)
            {
                Node *temp_node = static_cast<Node *>(node->items[i]);
                cout << string(4 * depth, ' ') << "Branch: Box = [";
                for (size_t j = 0; j < dimensions; j++)
                {
                    cout << "(" << temp_node->box.min_edges[j] << ", " << temp_node->box.max_edges[j] << ") ";
                }
                cout << "]" << endl;
                print_tree(temp_node, depth + 1);
            }
        }
    }

    void visualize_tree(Visualizer *visualizer, Node *node, int depth)
    {
        if (node->hasleaves)
        {
            for (size_t i = 0; i < node->items.size(); i++)
            {
                Leaf *temp_leaf = static_cast<Leaf *>(node->items[i]);
                // cout << string(4 * depth, ' ') << "Leaf: Value = " << temp_leaf->value << ", Box = [";
                for (size_t j = 0; j < dimensions; j++)
                {
                    // cout << "(" << temp_leaf->box.min_edges[j] << ", " << temp_leaf->box.max_edges[j] << ") ";
                    int x = temp_leaf->box.min_edges[0];
                    int y = temp_leaf->box.min_edges[1];
                    int z = temp_leaf->box.min_edges[2];
                    visualizer->drawPoint(x, y, z);
                }
                cout << "]" << endl;
            }
        }
        else
        {
            cout << string(4 * depth, ' ') << "Node:" << endl;
            for (size_t i = 0; i < node->items.size(); i++)
            {
                Node *temp_node = static_cast<Node *>(node->items[i]);
                cout << string(4 * depth, ' ') << "Branch: Box = [";
                int x, y, z, w, h, d;
                // for (size_t j = 0; j < dimensions; j++)
                // {
                //     cout << "(" << temp_node->box.min_edges[j] << ", " << temp_node->box.max_edges[j] << ") ";
                // }
                x = temp_node->box.min_edges[0];
                y = temp_node->box.min_edges[1];
                z = temp_node->box.min_edges[2];
                w = temp_node->box.max_edges[0] - temp_node->box.min_edges[0];
                h = temp_node->box.max_edges[1] - temp_node->box.min_edges[1];
                d = temp_node->box.max_edges[2] - temp_node->box.min_edges[2];
                visualizer->drawCube(x, y, z, w, h, d);
                cout << "]" << endl;
                visualize_tree(visualizer, temp_node, depth + 1);
            }
        }
    }

    /*
    La sección `private` de la clase contiene variables miembro
    que son específicas de la instancia de la clase `RStarTree`.
    Aquí está la explicación de cada una:

    - `unordered_set<int> used_deeps;`: Esta variable almacena
    los límites (profundidades) utilizados durante la inserción
     actual en el árbol. Se utiliza para llevar un registro de
     las profundidades que se han empleado durante el proceso
     de inserción en el árbol. Esto puede ser útil para ciertos
     métodos o para evitar repeticiones innecesarias de
     profundidades durante ciertas operaciones.

    - `Node *tree_root{nullptr};`: Es un puntero que apunta al
    nodo raíz del árbol R*-Tree. Este puntero es esencial para
    acceder a toda la estructura del árbol. Inicializado como
    `nullptr`, se espera que se asigne el nodo raíz cuando se
    inserten los primeros elementos en el árbol.

    - `size_t size_{0};`: Esta variable miembro mantiene un
    registro del número total de hojas en el árbol R*-Tree.
    Esencialmente, representa la cantidad de elementos (leaves)
    almacenados en el árbol. Inicializada en 0 para indicar
    que el árbol está vacío al inicio.

    Estas variables son fundamentales para el funcionamiento y
    seguimiento de la estructura del árbol R*-Tree, desde
    mantener el conteo de elementos hasta el seguimiento de
    profundidades utilizadas durante las inserciones.
    */
public:
    unordered_set<int>
        used_deeps; //<boundaries used during the current insertion
    Node *tree_root{nullptr};
    size_t size_{0}; //<number of leaves
};
