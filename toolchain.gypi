{
  'target_defaults': {
  	'defines': [
      'BOOST_ALL_NO_LIB',
      'BOOST_SYSTEM_NO_DEPRECATED',
      'BOOST_ERROR_CODE_HEADER_ONLY',
	  ],
	  'default_configuration': 'Debug',
    'conditions': [
      ['OS=="win"', {
        'defines': [
          'WIN32',
          '_WIN32_WINNT=0x1000',
        ],
        'msvs_windows_sdk_version': 'v10.0',
        'msvs_settings': {
          'VCCLCompilerTool': {
            'AdditionalOptions': ['/std:c++latest', '/FS', '/bigobj'],
          },
        },
      }],
      ['OS=="linux"', {
        'cflags_cc': [
          '-std=c++1z',
        ],
        'cflags': [
          # Warnings
          '-Weverything',
			    # Intended behavior
          '-Wno-pragma-pack-suspicious-include',
			    # We target C++17 + Networking TS
          '-Wno-c++11-extensions', 
          '-Wno-c++11-long-long', 
          '-Wno-c++98-compat', 
          '-Wno-c++98-compat-pedantic', 
			    # Stupid warnings
          '-Wno-unknown-warning-option',
          '-Wno-documentation', 
          '-Wno-documentation-unknown-command', 
          '-Wno-newline-eof', 
			    #  Protobuf generates these
			    '-Wno-zero-as-null-pointer-constant',
          '-Wno-unused-function',
			    # Net-TS impl generates these
          '-Wno-unused-local-typedef',
          '-Wno-disabled-macro-expansion',
          '-Wno-old-style-cast',
          # third party issues are not relevant for us
			    '--system-header-prefix=google/protobuf',
			    '--system-header-prefix=boost',
			    '--system-header-prefix=cryptopp',
			    '--system-header-prefix=zlib',
          #
          '-Wno-non-virtual-dtor',
          '-Wno-gnu-anonymous-struct',
          '-Wno-nested-anon-types',
          '-Wno-shadow', 
          '-Wno-undef', 
          '-Wno-implicit-fallthrough', 
          '-Wno-padded', 
          '-Wno-conversion', 
          '-Wno-undefined-reinterpret-cast', 
          '-Wno-weak-vtables', 
          '-Wno-deprecated', 
          '-Wno-extra-semi', 
          '-Wno-switch-enum', 
          '-Wno-global-constructors', 
          '-Wno-exit-time-destructors', 
          '-Wno-reserved-id-macro', 
          '-Wno-sign-conversion', 
          '-Wno-covered-switch-default', 
          '-Wno-reorder', 
			    '-Wno-double-promotion',
			    '-Wno-missing-prototypes', 
			    '-Wno-missing-variable-declarations',
        ],
			  'ldflags': [
				  '-lpthread',
			  ]
      }]
	  ],
	  'msvs_configuration_platform': 'x64',  
    'configurations': {
      'Debug': {
        'defines': [
          'DEBUG',
        ],
      },  
      'Release': {
        'conditions': [
          ['OS=="linux"', {
            'cflags': [
			        '-O2',
              '-fvisibility=hidden',
              '-fvisibility-inlines-hidden',
              '-ffunction-sections',
              '-fdata-sections',
			      ],
            'ldflags': [
              '-g',
              '-Wl,--gc-sections',
              '-fvisibility=hidden',
              '-fvisibility-inlines-hidden',
            ]
          }],
          ['OS=="win"', {
            'msvs_settings': {
              'VCCLCompilerTool': {
                'Optimization': '2',
                'InlineFunctionExpansion': '2',
                'EnableIntrinsicFunctions': 'true',
                'FavorSizeOrSpeed': '0',
                'StringPooling': 'true',
                'WholeProgramOptimization': 'true',  # /GL
              },
              'VCLinkerTool': {
                'LinkIncremental': '1',
                'OptimizeReferences': '2',
                'EnableCOMDATFolding': '2',
                'LinkTimeCodeGeneration': '1',     
              },
            },
          }], 
        ],
      },
    },
    
  }
}