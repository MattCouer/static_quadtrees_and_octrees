#ifndef Z_LOCATIONAL_CODE_H
#define Z_LOCATIONAL_CODE_H

#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class ZLocationalCode : public RefCounted {
	GDCLASS(ZLocationalCode, RefCounted)

private:
    

protected:
    static void _bind_methods();

public:
    int64_t morton_code;
    int64_t depth;
    
    static Ref<ZLocationalCode> create(int64_t _morton_code, int64_t _depth);

    int64_t get_morton_code();
    void set_morton_code(int64_t p_code);
    int64_t get_depth();
    void set_depth(int64_t p_depth);

ZLocationalCode(int64_t _morton_code, int64_t _depth);
ZLocationalCode();
~ZLocationalCode();


};

}

#endif