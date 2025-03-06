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

    return f'let to_{to_type}(what: {from_type}) -> {to_type};'


if __name__ == '__main__':
    types: List[str] = [
        'i8', 'u8',
        'i16', 'u16',
        'i32', 'u32',
        'i64', 'u64',
        'f32', 'f64',
        'bool'
    ]

    with open('casts.oak', 'w', encoding='utf8') as f:
        f.write(
            '/*\nCasts in Oak\n*/\n\n'
            'pragma!("no_dialect");\n'
            'link!("std/casts.o");\n\n')

        for lhs in types:
            for rhs in types:
                if lhs == rhs:
                    continue

                sig: str = signature(lhs, rhs)
                f.write(sig)
                f.write('\n')
