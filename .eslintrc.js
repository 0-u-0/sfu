module.exports = {
  'env': {
    'node': true,
    'commonjs': true,
    'es6': true,
    'jest': true
  }, 
  'extends': 'eslint:recommended',
  'globals': {
    'Atomics': 'readonly',
    'SharedArrayBuffer': 'readonly'
  },
  'parserOptions': {
    'ecmaVersion': 'latest'
  },
  'rules': {
    'space-before-blocks': 'error',
    'object-curly-spacing': ['error', 'always'],
    'arrow-parens': [ 2, 'always' ],
    'arrow-spacing': 2, 
    'space-infix-ops': 'error',
    'comma-spacing': ['error', { 'before': false, 'after': true }],
    'semi': 'error',
    'key-spacing': ['error', { 'afterColon': true }],
    'keyword-spacing': ['error', { 'before': true }],
    'quotes': [ 2, 'single', { avoidEscape: true } ],
    'indent': ['error', 2],
    'no-unused-vars': ['error', { 'args': 'none' }],
    'default-param-last': ['error'],
    'no-shadow': 'error',
    'no-loop-func': 'error',
    'no-multi-spaces': ['error', { ignoreEOLComments: false }]
  }
};