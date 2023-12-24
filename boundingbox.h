#pragma once
#include <algorithm>
#include <limits>
#include <vector>

using namespace std;
enum class axis_type
{
    upper,
    lower
};


template <size_t dimensions>
struct RStarBoundingBox
{
    vector<double> max_edges, min_edges; //<borders
    
    RStarBoundingBox() : max_edges(dimensions), min_edges(dimensions)
    {
        for (size_t axis = 0; axis < dimensions; axis++)
        {
            max_edges[axis] = numeric_limits<int>::min();
            min_edges[axis] = numeric_limits<int>::max();
        }
    }
    ~RStarBoundingBox() = default;
    bool operator<(const RStarBoundingBox &rhs) const
    {
        if (min_edges == rhs.min_edges)
            return max_edges < rhs.max_edges;
        else
            return min_edges < rhs.min_edges;
    }
    bool operator==(const RStarBoundingBox &rhs) const
    {
        return max_edges == max_edges && min_edges == min_edges;
    }
    bool operator!=(const RStarBoundingBox &rhs) const
    {
        return !operator==(rhs);
    }


    /*
    La función `reset` en la estructura `RStarBoundingBox` se 
    encarga de restablecer los límites del cuadro delimitador 
    a sus valores predeterminados. 
    Aquí está el desglose de la función:

    1. `void reset()`: Esta función no toma argumentos y simplemente 
    restablece los límites del cuadro delimitador a sus valores iniciales.

    2. Dentro del bucle `for`, recorre cada eje (dimensión) del cuadro delimitador:

        - `max_edges[axis] = numeric_limits<int>::min();`: Establece 
        el límite máximo (`max_edges`) en el eje actual al valor mínimo 
        posible para un entero (`numeric_limits<int>::min()`). 
        Este valor se utiliza para restablecer el límite máximo a un 
        valor inicial que permitirá expandir el cuadro delimitador 
        cuando sea necesario.

        - `min_edges[axis] = numeric_limits<int>::max();`: Establece 
        el límite mínimo (`min_edges`) en el eje actual al valor máximo 
        posible para un entero (`numeric_limits<int>::max()`). 
        Este valor se utiliza para restablecer el límite mínimo a un 
        valor inicial que permitirá ajustar el cuadro delimitador 
        correctamente cuando sea necesario.

    3. Al final de la función, todos los límites del cuadro delimitador
     se restablecen a valores extremos opuestos (`min_edges` a su máximo
      posible y `max_edges` a su mínimo posible) en cada dimensión.

    En resumen, `reset` reinicia los límites del cuadro delimitador, 
    estableciéndolos a valores iniciales que permiten la expansión o 
    ajuste correcto del cuadro cuando se necesite contener o abarcar 
    áreas específicas en un contexto más grande.    
    */
    void reset()
    {
        for (size_t axis = 0; axis < dimensions; axis++)
        {
            max_edges[axis] = numeric_limits<int>::min();
            min_edges[axis] = numeric_limits<int>::max();
        }
    }


    /*
    La función `stretch` es responsable de ajustar los límites
     de un cuadro delimitador (`BoundingBox`) para acomodar 
     completamente otra área dentro de él. 
     Aquí está la explicación detallada:

    1. `void stretch(const RStarBoundingBox<dimensions> &other_box)`: 
    Esta función toma como argumento otro cuadro delimitador 
    (`BoundingBox`) al que se quiere ajustar el cuadro delimitador actual.

    2. Dentro del bucle `for`, recorre cada eje (dimensión) del cuadro delimitador:

        - `max_edges[axis] = max(max_edges[axis], other_box.max_edges[axis]);`: 
        Para cada eje (`axis`), actualiza el límite máximo (`max_edges`) 
        tomando el valor máximo entre el límite máximo actual y el 
        límite máximo del otro cuadro delimitador. Esto asegura que 
        el límite máximo sea lo suficientemente grande como para 
        contener completamente la otra área en ese eje.

        - `min_edges[axis] = min(min_edges[axis], other_box.min_edges[axis]);`: 
        De manera similar, actualiza el límite mínimo (`min_edges`) 
        tomando el valor mínimo entre el límite mínimo actual y el 
        límite mínimo del otro cuadro delimitador. Esto asegura 
        que el límite mínimo sea lo suficientemente pequeño como
         para contener completamente la otra área en ese eje.

    3. Al final de la función, el cuadro delimitador actual (`this`)
     se ajusta para acomodar completamente el otro cuadro delimitador,
      asegurando que sus límites abarquen toda el área del otro cuadro.

    En resumen, `stretch` se encarga de expandir o ajustar los límites 
    del cuadro delimitador actual para asegurar que otro cuadro delimitador 
    (otra área) esté completamente contenido dentro de él. 
    Esto se hace actualizando los límites máximos y mínimos 
    en cada dimensión para englobar completamente la otra área.
    */
    void stretch(const RStarBoundingBox<dimensions> &other_box)
    {
        for (size_t axis = 0; axis < dimensions;
             axis++)
        { // Selects borders so that another area can be accommodated
            max_edges[axis] = max(max_edges[axis], other_box.max_edges[axis]);
            min_edges[axis] = min(min_edges[axis], other_box.min_edges[axis]);
        }
    }



    /*
    La función `is_intersected` dentro de la clase `RStarBoundingBox` 
    determina si dos cajas delimitadoras (`RStarBoundingBox`) se 
    intersectan entre sí. Aquí está su explicación:

    - `bool is_intersected(const RStarBoundingBox<dimensions> &other_box) const`: 
    Esta función toma una caja delimitadora (`other_box`) como 
    argumento y verifica la intersección con la caja actual.

    La función opera de la siguiente manera:

    - Llama a otra función dentro de la misma clase, `overlap(other_box)`, 
    para calcular el área de intersección entre las dos cajas.
    - Verifica si el área de intersección calculada (`overlap(other_box)`) 
    es mayor que cero.
        - Si el área de intersección es mayor que cero, significa que 
        hay una superposición entre las cajas delimitadoras.
        - Devuelve `true` para indicar que las cajas se intersectan.
        - De lo contrario, si el área de intersección es cero o negativa,
         devuelve `false` para indicar que no hay superposición entre las cajas.

    En resumen, esta función actúa como un verificador de intersección 
    entre dos cajas delimitadoras, devolviendo `true` si hay una 
    intersección y `false` si no la hay. Esto es útil al determinar 
    si dos áreas definidas por las cajas delimitadoras se superponen 
    entre sí.
    */
    bool is_intersected(const RStarBoundingBox<dimensions> &other_box) const
    {
        if (overlap(other_box) > 0)
            return true; // Returns true if the intersection is greater than 0
        else
            return false;
    }


    /*
    `margin()` calcula el margen o borde 
    alrededor de la caja delimitadora (`BoundingBox`). 
    El margen es la suma de las longitudes de los lados de 
    la caja delimitadora a lo largo de cada dimensión.

    Aquí está el desglose de la función:

    - `int ans = 0;`: Se inicializa la variable `ans` como cero, 
    la cual almacenará el cálculo total del margen.

    - El bucle `for` itera a través de cada dimensión del espacio 
    (considerando que la caja delimitadora puede ser multidimensional):
        - `for (size_t axis = 0; axis < dimensions; axis++)`: 
        Itera a través de cada dimensión del espacio.
        - `ans += max_edges[axis] - min_edges[axis];`: 
        Para cada dimensión, se calcula la longitud del lado 
        restando el valor mínimo del borde al valor máximo 
        del borde en esa dimensión. Esto se hace para todas 
        las dimensiones y se suma al total del margen (`ans`).

    Finalmente, se devuelve la suma total acumulada de los 
    bordes de la caja delimitadora en todas las dimensiones, 
    lo que representa el margen total alrededor de la caja.
    */
    int margin() const
    {
        int ans = 0;
        for (size_t axis = 0; axis < dimensions;
             axis++)
        { // Calculates the sum of the umbrellas
            ans += max_edges[axis] - min_edges[axis];
        }
        return ans;
    }



    /*
    `area()` calcula el área de la caja delimitadora en
     un espacio multidimensional. El área en este contexto 
     se refiere al producto de las longitudes de los lados 
     de la caja delimitadora a lo largo de cada dimensión.

    Aquí está el desglose de la función:

    - `int ans = 1;`: Se inicializa la variable `ans` como uno. 
    Esta variable almacenará el cálculo total del área.

    - El bucle `for` itera a través de cada dimensión del espacio:
        - `for (size_t axis = 0; axis < dimensions; axis++)`: 
        Itera a través de cada dimensión del espacio.
        - `ans *= max_edges[axis] - min_edges[axis];`: 
        Para cada dimensión, se calcula la longitud del lado
         multiplicando el valor mínimo del borde por el valor
          máximo del borde en esa dimensión. Esto se hace para
           todas las dimensiones y se multiplica al total del 
           área (`ans`).

    Finalmente, se devuelve el producto total de las longitudes 
    de los lados de la caja delimitadora en todas las dimensiones,
     lo que representa el área total de la caja en el espacio multidimensional.
    */
    int area() const
    { // Calculates surface area
        int ans = 1;
        for (size_t axis = 0; axis < dimensions; axis++)
        {
            ans *= max_edges[axis] - min_edges[axis];
        }
        return ans;
    }



    /*
    La función `overlap` calcula el área de intersección 
    entre dos cajas delimitadoras en un espacio de múltiples 
    dimensiones. Aquí está su explicación detallada:

    - `int overlap(const RStarBoundingBox<dimensions> &other_box) const`: 
    Esta función toma otra caja delimitadora (`other_box`) como 
    argumento y calcula el área de intersección con la caja 
    delimitadora actual.

    El cálculo se realiza de la siguiente manera:

    1. Se inicializa una variable `ans` (respuesta) en 1, que 
    se utilizará para almacenar el área de intersección final.

    2. Se itera sobre cada dimensión de las cajas delimitadoras 
    a través de un bucle `for`:

        - Se definen variables `x1`, `x2`, `y1`, y `y2` para 
        representar los límites de las cajas en la dimensión 
        actual. `x1` y `x2` son los límites de la caja actual, 
        mientras que `y1` e `y2` son los límites de la otra 
        caja (`other_box`) en la misma dimensión.

        - Se comparan estos límites para determinar la 
        intersección entre las cajas en esa dimensión específica.

    3. A partir de las comparaciones entre límites, se calcula 
    el área de intersección en esa dimensión, multiplicando 
    la longitud del segmento de intersección por `ans`. Las 
    condiciones verifican si hay intersección y cómo se 
    superponen las cajas en esa dimensión específica.

    4. Si en algún punto no hay intersección entre las cajas 
    en una dimensión determinada, el valor de `ans` se establece 
    en 0, lo que indica que el área de intersección total es cero.

    5. Al final del bucle, se devuelve `ans`, que representa 
    el área de intersección total entre las cajas delimitadoras 
    en todas las dimensiones.

    En resumen, esta función `overlap` realiza cálculos complejos 
    basados en límites de cajas en cada dimensión para determinar 
    el área de intersección total entre dos cajas delimitadoras en 
    un espacio de múltiples dimensiones.
    */
    int overlap(const RStarBoundingBox<dimensions> &other_box) const
    {
        int ans = 1;
        for (size_t axis = 0; axis < dimensions; axis++)
        {
            int x1 = min_edges[axis];           // lower limit
            int x2 = max_edges[axis];           // larger limit
            int y1 = other_box.min_edges[axis]; // smallest limit
            int y2 = other_box.max_edges[axis]; // larger limit

            if (x1 < y1)
            {
                if (y1 < x2)
                {
                    if (y2 < x2)
                    {
                        ans *= (y2 - y1);
                    }
                    else
                    {
                        ans *= (x2 - y1);
                    }
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                if (y2 > x1)
                {
                    if (y2 > x2)
                    {
                        ans *= (x2 - x1);
                    }
                    else
                    {
                        ans *= (y2 - x1);
                    }
                }
                else
                {
                    return 0; // if it doesn't fit into any condition.
                }
            }
        }
        return ans;
    }



    /*
    `dist_between_centers` calcula la distancia al cuadrado 
    entre el centro de la caja delimitadora actual y el 
    centro de otra caja delimitadora en un espacio multidimensional. 
    Específicamente, este método se utiliza para calcular 
    la distancia entre los centros de dos cajas delimitadoras.

    Aquí está el desglose de la función:

    - `int ans = 0;`: Se inicializa la variable `ans` como cero. 
    Esta variable almacenará la suma de los cuadrados de 
    las diferencias en cada dimensión entre los centros de 
    las cajas delimitadoras.

    - El bucle `for` itera a través de cada dimensión del espacio:
        - `for (size_t axis = 0; axis < dimensions; axis++)`: 
        Itera a través de cada dimensión del espacio.
        - `int d = ((max_edges[axis] + min_edges[axis]) -
         (other_box.max_edges[axis] + other_box.min_edges[axis])) / 2;`: 
         Para cada dimensión, calcula la distancia entre los
          centros de las cajas delimitadoras en esa dimensión. 
          Esta distancia se calcula tomando la diferencia 
          entre los centros de las dos cajas y dividiéndola por 2.
        - `ans += d * d;`: Se agrega el cuadrado de esta 
        distancia a la suma total de cuadrados (`ans`).

    Finalmente, se devuelve la suma de los cuadrados de las 
    diferencias entre los centros de las cajas delimitadoras 
    en cada dimensión, lo que representa la distancia al
     cuadrado entre los centros de las cajas. Esta distancia
      se devuelve al cuadrado para evitar la necesidad de
       realizar una operación de raíz cuadrada, lo que puede 
       ahorrar en términos de precisión y tiempo de cálculo.
    */
    double dist_between_centers(const RStarBoundingBox<dimensions> &other_box) const
    {
        // The result is the distance squared so as not to lose accuracy from the
        // square root.
        int ans = 0;
        for (size_t axis = 0; axis < dimensions; axis++)
        {
            int d = ((max_edges[axis] + min_edges[axis]) -
                     (other_box.max_edges[axis] + other_box.min_edges[axis])) /
                    2;
            ans += d * d;
        }
        return ans;
    }



    /*
     `value_of_axis`, devuelve el valor 
    de borde en un eje específico de la caja delimitadora.

    - `int value_of_axis(const int axis, const axis_type type) const`: 
    Esta función toma dos argumentos: `axis`, que representa
     la dimensión o eje para el que se quiere obtener el 
     valor del borde, y `type`, que indica si se busca el 
     valor del borde inferior (`axis_type::lower`) o 
     superior (`axis_type::upper`) en ese eje.

    - Dentro de la función:
        - `if (type == axis_type::lower)`: Verifica si 
        el tipo de borde que se está buscando es el borde 
        inferior en el eje dado.
            - `return min_edges[axis];`: Si `type` es 
            `axis_type::lower`, devuelve el valor del 
            borde inferior en el eje especificado por `axis`.

        - `else`: En caso contrario (si el tipo de borde es 
        `axis_type::upper` o cualquier otro valor diferente de `axis_type::lower`):
            - `return max_edges[axis];`: Devuelve el valor
             del borde superior en el eje especificado por `axis`.

    En resumen, esta función devuelve el valor del borde 
    inferior o superior, según lo especificado por `type`,
     en el eje o dimensión indicado por `axis` en la caja delimitadora.
    */
    int value_of_axis(const int axis, const axis_type type) const
    {
        if (type == axis_type::lower)
        {
            return min_edges[axis];
        }
        else
        {
            return max_edges[axis];
        }
    }
    
};