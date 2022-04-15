#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <utility>

template <typename Type>
class ArrayPtr {
public:
    // Инициализирует ArrayPtr нулевым указателем
    ArrayPtr() = default;

    // Создаёт в куче массив из size элементов типа Type.
    // Если size == 0, поле raw_ptr_ должно быть равно nullptr
    explicit ArrayPtr(size_t size) {
        if(size > 0) {
            raw_ptr_ = new Type[size]{};
        }
    }

    // Конструктор из сырого указателя, хранящего адрес массива в куче либо nullptr
    explicit ArrayPtr(Type* raw_ptr) noexcept 
        : raw_ptr_(raw_ptr) {
    }

    // Запрещаем копирование
    ArrayPtr(const ArrayPtr& other) = delete;

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    // Запрещаем присваивание
    ArrayPtr& operator=(const ArrayPtr& other) = delete;

    //movable constructor
    ArrayPtr(ArrayPtr&& other) 
    {
        //указатель в other становится nullptr
        raw_ptr_ = std::exchange(other.raw_ptr_, nullptr);
    }

    //movable assignment
    ArrayPtr& operator=(ArrayPtr&& rhs) {
        raw_ptr_ = std::exchange(rhs.raw_ptr_, raw_ptr_);

        //std::swap(raw_ptr_, rhs.raw_ptr_); // right - alternative

        //raw_ptr_ = std::exchange(rhs.raw_ptr_, nullptr); //wrong - memory leakage

        //std::exchange(rhs.raw_ptr_, raw_ptr_); Непонятен комментарий! Данная строка вызывает segmentation fault во втором тесте.
        
        // вот, что было в уроке:
        //     Кролик переместился, белый цвет остался. 
        //     Для программиста ничего страшного не произошло. Объект остался в валидном состоянии. 
        //     Но для фокусника этот результат выглядит странно. Хорошим тоном будет задать значение цвета, которое объект будет получать по умолчанию после перемещения. 
        //     Пусть перемещённый кролик становится невидимым благодаря функции exchange:
            
                // Rabbit(Rabbit&& other) {
                //     color_ = exchange(other.color_, Color::INVISIBLE);
                // }

                // Rabbit& operator=(Rabbit&& other) {
                //     color_ = exchange(other.color_, Color::INVISIBLE);
                //     return *this;
                // } 

        // Теперь точно известно, что в magic_hat после перемещения будет лежать невидимый кролик.
        // В итоговом проекте спринта вы будете добавлять поддержку move-семантики для своего вектора и списка. 
        // Функция exchange будет полезна для гарантии, что указатель на данные не остался в перемещённом объекте, а был заменён на nullptr.


        return *this;
    }

    // Прекращает владением массивом в памяти, возвращает значение адреса массива
    // После вызова метода указатель на массив должен обнулиться
    [[nodiscard]] Type* Release() noexcept {
        Type* return_ptr = raw_ptr_;
        raw_ptr_ = nullptr;
        return return_ptr;
    }

    // Возвращает ссылку на элемент массива с индексом index
    Type& operator[](size_t index) noexcept {
        return *(raw_ptr_ + index);
    }

    // Возвращает ссылку на элемент массива с индексом index
    const Type& operator[](size_t index) const noexcept {
        return *(raw_ptr_ + index);
    }

    explicit operator bool() const {
        if(raw_ptr_ == nullptr) {
            return false;
        }
        else {
            return true;
        }
    }

    // Возвращает значение сырого указателя, хранящего адрес начала массива
    Type* Get() const noexcept {
        return raw_ptr_;
    }

    void swap(ArrayPtr& other) noexcept {
        // Type* temp = other.Get();
        // other.raw_ptr_ = raw_ptr_;
        // raw_ptr_ = temp;

        std::swap(raw_ptr_, other.raw_ptr_);
    }

private:
    Type* raw_ptr_ = nullptr;
};