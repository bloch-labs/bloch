const ALLOWED_TYPES = [
  'feat',
  'fix',
  'bugfix',
  'hotfix',
  'chore',
  'docs',
  'refactor',
  'perf',
  'test',
  'build',
  'ci',
  'style',
  'revert'
];

const typePattern = ALLOWED_TYPES.join('|');

module.exports = {
  extends: ['@commitlint/config-conventional'],
  rules: {
    'type-enum': [2, 'always', ALLOWED_TYPES],
    'subject-case': [0],
    'scope-case': [2, 'always', ['lower-case', 'kebab-case']],
    'header-max-length': [2, 'always', 100],
    'header-match-pattern': [
      2,
      'always',
      `^(?:${typePattern})(?:\([a-z0-9\-\/]+\))?!?: .+ \(#\\d+\)$`
    ],
  },
  ignores: [
    (message) => message.startsWith('Merge'),
    (message) => /chore: release/i.test(message),
    (message) => message.startsWith('chore(') && message.includes('release'),
  ],
};
