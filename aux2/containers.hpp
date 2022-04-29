#ifndef AUX2_COINTAINERS_HPP
#define AUX2_COINTAINERS_HPP

#include <vector>
#include <list>


/* invertir
 * 
 * Invierte el orden de los elementos de una lista.
 */

std::list<int> invertir(const std::list<int>& l);

/* cola de rendering.
 * 
 * La cola debe almacenar todas las draw calls creadas con la función enqueue().
 * La función drawCalls() debe retornar un vector que contenga las draw calls.
 * La función sort() debe ordenar las drawcalls en cola de acuerdo al shader que utilizan.
 * La función clear() debe reestablecer el estado de la cola (vaciarla).
 */

using Object = int;
using Shader = int;

struct DrawCall {
    Object object;
    Shader shader;
};

class RenderQueue {
public:
    void enqueue(Object obj, Shader shader);

    const std::vector<DrawCall>& drawCalls() const;

    void clear();

    void sort();
private:
};

#endif//AUX2_COINTAINERS_HPP