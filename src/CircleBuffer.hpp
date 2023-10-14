#include <cstdint>

template<typename Type>
struct CircleBuffer {
    struct Buffer {
        std::uint32_t size;
        std::uint32_t read{}, write{};
        Buffer(std::uint32_t size) : size{size} {}
    };

    CircleBuffer(std::uint32_t size) {
        owning = true;
        auto raw = new uint8_t[sizeof(Buffer) + sizeof(Type) * size];
        buffer = new (raw) Buffer(size);
        for (std::uint32_t i = 0; i < size; ++i)
            new (data() + i) Type();
    }

    CircleBuffer() = default;

    void reset(void* buffer) {this->buffer = reinterpret_cast<Buffer*>(buffer);}

    ~CircleBuffer() {
        if (!buffer || !owning)
            return;
        for (std::uint32_t i = 0; i < buffer->size; ++i)
            data()[i].~Type();
        auto raw = reinterpret_cast<uint8_t*>(buffer);
        delete[] raw;
        buffer = nullptr;
    }

    Type* data() {
        return buffer ? reinterpret_cast<Type*>(buffer + 1) : nullptr;
    }

    std::size_t size() {return buffer ? buffer->size : 0;}

    bool empty() {return buffer ? buffer->read == buffer->write : true;}

    Type& pop() {
        auto pos = buffer->read++;
        if (buffer->read >= buffer->size)
            buffer->read = 0;
        return data()[pos];
    }

    void push(const Type& value) {
        auto write = buffer->write;
        data()[write] = value;
        ++write;
        if (write >= buffer->size)
            write = 0;
        buffer->write = write;
        if (write == buffer->read) {
            buffer->read++;
            if (buffer->read >= buffer->size)
                buffer->read = 0;
        }
    }

    Buffer* get() {return buffer;}

private:
    Buffer* buffer{};
    bool owning{};
};
