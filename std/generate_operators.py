'''
Generates the `std/operators.oak` and `std/operators.c` files.
'''

import sys
from os import path
from typing import List, Dict, Tuple
from shutil import move
from subprocess import check_output, run


def oak2c(signature: str) -> str:
    '''
    Calls the oak2c CLI resource, returning what it outputs.
    The return will be a semicolon-terminated C function
    signature.
    :param signature: An Oak signature
    :returns: A C signature
    '''

    return check_output(['oak2c', signature]).decode('utf-8')


def main() -> int:
    '''
    Main function for generation. Run this INSIDE the `std` dir.
    :returns: 0 on success, nonzero on failure.
    '''

    # The basic atomic types to iterate over
    types: List[str] = [
        'i8', 'u8',
        'i16', 'u16',
        'i32', 'u32',
        'i64', 'u64',
        'int', 'uint',
        'f32', 'f64',
        'bool'
    ]

    # Homogenous const binary operations
    binary_operations: List[Tuple[str, str]] = [
        ('Add', '+'),
        ('Sub', '-'),
        ('Mult', '*'),
        ('Div', '/'),
        ('Mod', '%'),
    ]

    # Binary comparisons
    binary_comparisons: List[Tuple[str, str]] = [
        ('Eq', '=='),
        ('Neq', '!='),
        ('Less', '<'),
        ('Leq', '<='),
        ('Great', '>'),
        ('Greq', '>='),
    ]

    # Mutable unary operations
    unary_operations: List[Tuple[str, str]] = [
        ('Incr', '++'),
        ('Decr', '--'),
    ]

    # Homogenous first-arg-mutable binary operations
    mut_binary_operations: List[Tuple[str, str]] = [
        ('Copy', '='),
        ('AddEq', '+='),
        ('SubEq', '-='),
        ('MultEq', '*='),
        ('DivEq', '/='),
        ('ModEq', '%='),
    ]

    # Cases to exclude
    exclude: Dict[str, List[str]] = {
        'f32': ['Mod', 'ModEq', 'Incr', 'Decr'],
        'f64': ['Mod', 'ModEq', 'Incr', 'Decr'],
        'bool': ['Less', 'Leq', 'Great',
                 'Greq', 'AddEq', 'SubEq',
                 'MultEq', 'DivEq', 'ModEq',
                 'Incr', 'Decr'],
    }

    # Filepaths
    target_oak: str = 'operators.oak'
    target_c: str = 'operators.c'

    # Make backups
    if path.exists(target_oak):
        print('Saving backup in /tmp/')
        move(target_oak, '/tmp/' + target_oak)
    if path.exists(target_c):
        print('Saving backup in /tmp/')
        move(target_c, '/tmp/' + target_c)

    # Open files
    with (open(target_oak, 'w', encoding='utf8') as oak,
          open(target_c, 'w', encoding='utf8') as c):

        # Headers
        oak.write(
            '/*\n'
            'Standard operators for Oak: Oak binding\n'
            '*/\n\n'
            'pragma!(\"no_dialect\");\n'
            'link!(\"std/operators.o\");\n\n'
        )
        c.write(
            '/*\n'
            'Standard operators for Oak: C binding\n'
            '*/\n\n'
            '#include "std_oak_header.h"\n'
            '#include <string.h>\n\n'
        )

        # Const binary operators
        for t in types:
            signature: str = ''
            for oak_name, c_operator in binary_operations:
                if t in exclude and oak_name in exclude[t]:
                    continue

                if signature:
                    signature += ', '
                signature += oak_name

                c_signature: str = oak2c(
                    f'let {oak_name}(l: {t}, '
                    f'r: {t}) -> {t};')

                c_definition: str = \
                    c_signature[:-2] + '{\n  return l ' + \
                    c_operator + ' r;\n}\n\n'

                c.write(c_definition)

            # Write to Oak
            if signature:
                if len(f'let {signature}(l: {t}, '
                       f'r: {t}) -> {t};\n') > 64:
                    oak.write(f'let {signature}\n'
                              f'  (l: {t}, r: {t}) -> {t};\n')
                else:
                    oak.write(f'let {signature}(l: {t}, '
                              f'r: {t}) -> {t};\n')

        # Const binary comparisons
        for t in types:
            signature: str = ''
            for oak_name, c_operator in binary_comparisons:
                if t in exclude and oak_name in exclude[t]:
                    continue

                if signature:
                    signature += ', '
                signature += oak_name

                c_signature: str = oak2c(
                    f'let {oak_name}(l: {t}, '
                    f'r: {t}) -> bool;')

                c_definition: str = \
                    c_signature[:-2] + '{\n  return l ' + \
                    c_operator + ' r;\n}\n\n'

                c.write(c_definition)

            # Write to Oak
            if signature:
                if len(f'let {signature}(l: {t}, '
                       f'r: {t}) -> bool;\n') > 64:
                    oak.write(f'let {signature}\n'
                              f'(l: {t}, r: {t}) -> bool;\n')
                else:
                    oak.write(f'let {signature}(l: {t}, '
                              f'r: {t}) -> bool;\n')

        # Mutable binary operators
        for t in types:
            signature: str = ''
            for oak_name, c_operator in mut_binary_operations:
                if t in exclude and oak_name in exclude[t]:
                    continue

                if signature:
                    signature += ', '
                signature += oak_name

                c_signature: str = oak2c(
                    f'let {oak_name}(self: ^{t}, '
                    f'what: {t}) -> {t};')

                c_definition: str = \
                    c_signature[:-2] + '{\n  *self ' + \
                    c_operator + ' what;\n' + \
                    '  return *self;\n}\n\n'

                c.write(c_definition)

            # Write to Oak
            if signature:
                if len(f'let {signature}(self: ^{t}, '
                       f'what: {t}) -> {t};\n') > 64:
                    oak.write(f'let {signature}\n'
                              f'  (self: ^{t}, '
                              f'what: {t}) -> {t};\n')
                else:
                    oak.write(f'let {signature}(self: ^{t}, '
                              f'what: {t}) -> {t};\n')

        # Mutable unary operators
        for t in types:
            signature: str = ''
            for oak_name, c_operator in unary_operations:
                if t in exclude and oak_name in exclude[t]:
                    continue

                if signature:
                    signature += ', '
                signature += oak_name

                c_signature: str = oak2c(
                    f'let {oak_name}(self: ^{t}) -> {t};')

                if c_operator:
                    c.write(c_signature[:-2] + '{\n  ' +
                            c_operator + '(*self);\n' +
                            '  return *self;\n}\n\n')

                else:
                    c.write(c_signature[:-2] +
                            '{\n  return *self;\n}\n\n')

            # Write to Oak
            if signature:
                if len(f'let {signature}'
                       f'(self: ^{t}) -> {t};\n') > 64:
                    oak.write(f'let {signature}\n'
                              f'(self: ^{t}) -> {t};\n')
                else:
                    oak.write(f'let {signature}'
                              f'(self: ^{t}) -> {t};\n')

        # Special cases
        oak.write(
            'let Eq, Neq(l: []i8, r: []i8) -> bool;\n'
            'let Orr, Andd(l: bool, r: bool) -> bool;\n'
            'let AnddEq, OrrEq(self: ^bool, other: bool'
            ') -> bool;\n'
            'let Not(x: bool) -> bool;\n')

        c.write(
            oak2c('let Eq(l: []i8, r: []i8) -> bool;')[:-2]
            + '{\n  return strcmp((char *)l, '
            + '(char *)r) == 0;\n}\n\n')
        c.write(
            oak2c('let Neq(l: []i8, r: []i8) -> bool;')[:-2]
            + '{\n  return strcmp((char *)l, '
            + '(char *)r) != 0;\n}\n\n')
        c.write(
            oak2c('let Orr(l: bool, r: bool) -> bool;')[:-2]
            + '{\n  return l || r;\n}\n\n')
        c.write(
            oak2c(
                'let Andd(l: bool, r: bool) -> bool;')[:-2]
            + '{\n  return l && r;\n}\n\n')
        c.write(
            oak2c(
                'let AnddEq(self: ^bool, '
                'other: bool) -> bool;')[:-2] +
            '{\n  *self &= other;\nreturn *self;\n}\n\n')
        c.write(
            oak2c(
                'let OrrEq(self: ^bool, '
                'other: bool) -> bool;')[:-2] +
            '{\n  *self &= other;\nreturn *self;\n}\n\n')
        c.write(
            oak2c('let Not(x: bool) -> bool;')[:-2]
            + '{\n  return !x;\n}\n\n')

    run(['clang-format', '-i', target_c], check=True)

    return 0


if __name__ == '__main__':
    sys.exit(main())
