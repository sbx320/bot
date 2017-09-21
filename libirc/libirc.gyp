{
	'targets': [
	{
		'target_name': 'libirc',
		'dependencies': [
		],
		'type': 'static_library',
		'direct_dependent_settings': {
          'include_dirs': [
			'.',
		  ],
        },
		'include_dirs': [
			'.',
			'../libsteam/vendor/',
		],
		'sources': [
			'<!@pymod_do_main(glob-files ./**/*.cpp)',
			'<!@pymod_do_main(glob-files ./**/*.h)',
		]
	},
	]
}
