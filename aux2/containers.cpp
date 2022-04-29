#include "containers.hpp"


// RenderQueue

void RenderQueue::enqueue(Object object, Shader shader) {
    return;
}

const std::vector<DrawCall>& RenderQueue::drawCalls() const{
    static std::vector<DrawCall> vec {};
    return vec;
}

void RenderQueue::clear() {

}

void RenderQueue::sort() {

}


// invertir

std::list<int> invertir(const std::list<int>& l) {
    return {};
}