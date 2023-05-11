//
// Created by Alex Shchelochkov on 19.07.2022.
//
#pragma once

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <sstream>

namespace mllib {
    template<typename T>
    class Matrix {
    private:
        /*
         * True     -> row by row
         * False    -> column by column
         */
        bool order;
        size_t rows;
        size_t columns;
        T *data;
    public:
        Matrix();
        Matrix(size_t rows, size_t columns, bool order = true);
        Matrix(size_t rows, size_t columns, bool order, T value);
        Matrix(size_t rows, size_t columns, bool order, T *data);
        Matrix(const std::vector<std::vector<T>> &v);
        Matrix(Matrix &&m) noexcept;
        ~Matrix();
        Matrix &operator=(Matrix &&m) noexcept;
        T& operator() (size_t i, size_t j);
        T operator() (size_t i, size_t j) const;
        std::pair<const T *, const T *> operator[](size_t id) const;
        std::pair<size_t, size_t> size() const;
        bool empty() const;
        const T *at(size_t i, size_t j) const;
        void set(size_t i, size_t j, const T &new_val);
        std::string to_string() const;
    };
}
template<typename T>
mllib::Matrix<T>::Matrix() : order(true), rows(0), columns(0), data(nullptr) {}

template<typename T>
mllib::Matrix<T>::Matrix(size_t rows, size_t columns, bool order) : Matrix(rows, columns, order, T(0)) {}

template<typename T>
mllib::Matrix<T>::Matrix(size_t rows, size_t columns, bool order, T value) {
    this->rows = rows;
    this->columns = columns;
    this->order = order;
    this->data = new T[rows*columns];
    std::fill(this->data, this->data+(rows * columns), value);
}

template<typename T>
mllib::Matrix<T>::Matrix(size_t rows, size_t columns, bool order, T *data) {
    this->rows = rows;
    this->columns = columns;
    this->order = order;
    this->data = std::move(data);
}

template<typename T>
mllib::Matrix<T>::Matrix(const std::vector<std::vector<T>> &v) {
    this->rows = v.size();
    this->columns = v.empty() ? 0 : v.front().size();
    this->order = true;
    this->data = new T[rows*columns];
    for (size_t i = 0; i < rows; i++)
        for (size_t j = 0; j < columns; j++)
            data[i * columns + j] = v[i][j];
}

template<typename T>
mllib::Matrix<T>::Matrix(Matrix &&m) noexcept :
order(std::move(m.order)),
rows(std::move(m.rows)),
columns(std::move(m.columns)),
data(std::move(m.data))
{
    m.order = true;
    m.rows = 0;
    m.columns = 0;
    m.data = nullptr;
}

template<typename T>
mllib::Matrix<T>::~Matrix() {
    delete[] data;
}

template<typename T>
mllib::Matrix<T> &mllib::Matrix<T>::operator=(Matrix &&m) noexcept {
    this->rows = std::move(m.rows);
    this->columns = std::move(m.columns);
    this->order = std::move(m.order);
    this->data = std::move(m.data);
    m.order = true;
    m.rows = 0;
    m.columns = 0;
    m.data = nullptr;
    return *this;
}
template<typename T>
T& mllib::Matrix<T>::operator() (size_t i, size_t j) {
    size_t idx;
    if (order)
        idx = i * columns + j;
    else
        idx = j * rows + i;
    return data[idx];
}

template<typename T>
T mllib::Matrix<T>::operator() (size_t i, size_t j) const {
    size_t idx;
    if (order)
        idx = i * columns + j;
    else
        idx = j * rows + i;
    return data[idx];
};

template<typename T>
std::pair<const T *, const T *> mllib::Matrix<T>::operator[](size_t id) const {
    T *begin = data, *end;
    if (order) {
        begin += id * columns;
        end = begin + columns;
    } else {
        begin += id * rows;
        end = begin + rows;
    }
    return {begin, end};
}

template<typename T>
std::pair<size_t, size_t> mllib::Matrix<T>::size() const {
    return {rows, columns};
}

template<typename T>
bool mllib::Matrix<T>::empty() const {
    return rows * columns == 0;
}

template<typename T>
const T *mllib::Matrix<T>::at(size_t i, size_t j) const {
    size_t idx;
    if (order)
        idx = i * columns + j;
    else
        idx = j * rows + i;
    return &data[idx];
}

template<typename T>
void mllib::Matrix<T>::set(size_t i, size_t j, const T &new_val) {
    size_t idx;
    if (order)
        idx = i * columns + j;
    else
        idx = j * rows + i;
    data[idx] = new_val;
}

template<typename T>
std::string mllib::Matrix<T>::to_string() const {
    std::stringstream ss;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            size_t idx = 0;
            if (order)
                idx = i * columns + j;
            else
                idx = j * rows + i;
            ss << data[idx] << ' ';
        }
        ss << '\n';
    }
    return ss.str();
}