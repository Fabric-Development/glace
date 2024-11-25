project('glace', 'c', version: '0.1', license: 'AGPL-3.0-or-later')

if build_machine.system() == 'windows'
    error('libglace is not meant for windows users, consider moving to linux.')
endif

subdir('protocol')
subdir('glace')
