'''
Generates the `std/casts.oak` file
'''

from typing import List


def signature(from_type: str, to_type: str) -> str:
    '''
    Returns a single signature
    :param from_type: The source type
    :param to_type: The target type
    :returns: The string representing that signature in Oak
    '''

    return f'let to_{to_type}(what: {from_type}) -> {to_type};\n'

def implementation(src: str, tgt: str) -> str:
    '''
    Returns a single implementation
    :param src: The source type
    :param tgt: The target type
    :returns: The string representing that cast implementation
    in C
    '''

    return f'// to_{tgt}(what: {src}) -> {tgt}\n' \
        f'{tgt} to_{tgt}_FN_{src}_MAPS_{tgt}({src} what)' \
        ' {\n  return what;\n}\n\n'


if __name__ == '__main__':
    types: List[str] = [
        'i8', 'u8',
        'i16', 'u16',
        'i32', 'u32',
        'i64', 'u64',
        'f32', 'f64',
        'bool'
    ]

    with (open('casts.oak', 'w', encoding='utf8') as oak,
          open('casts.c', 'w', encoding='utf8') as c):
        oak.write(
            '/**\n * @brief Casts in Oak: Autogen\n */\n\n'
            'pragma!("no_dialect");\n'
            'link!("std/casts.o");\n\n')

        c.write('/**\n * @brief Casts for Oak: Autogen\n */\n\n'
                '#include "std_oak_header.h"\n\n')

        for lhs in types:
            for rhs in types:
                sig: str = signature(lhs, rhs)
                impl: str = implementation(lhs, rhs)
                oak.write(sig)
                c.write(impl)
