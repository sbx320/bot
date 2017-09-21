{
	'targets': [
	{
		'target_name': 'rd2lbot',
		'dependencies': [
			"libirc/libirc.gyp:libirc",
			"libsteam/libsteam.gyp:libsteam",
			"libsteam/libsteam.gyp:libdota2",
		],
		'type': 'executable',
		'include_dirs': [
			'src',
			'vendor/json/src',
			'vendor/beast/include',
			'libsteam/vendor/',
		],
		'sources': [
			'<!@pymod_do_main(glob-files src/**/*.cpp)',
			'<!@pymod_do_main(glob-files src/**/*.h)',
		],
		'msvs_settings': {
			'VCCLCompilerTool': {
				'AdditionalOptions': ['/bigobj'],
			},
			'VCLinkerTool': {
				'AdditionalDependencies': [
					'dbghelp.lib',
					'Shell32.lib',
					'User32.lib'
				],
			},
		},
		'conditions': [
			['OS=="win"', {
				'defines' : [
					'NOMINMAX',
				],
			}]
		]
	},
	]
}
