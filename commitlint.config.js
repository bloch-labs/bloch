const ALLOWED_TYPES = [
  'build',
  'chore',
  'ci',
  'docs',
  'feat',
  'fix',
  'perf',
  'refactor',
  'revert',
  'style',
  'test',
];

module.exports = {
  extends: ['@commitlint/config-conventional'],
  rules: {
    'type-enum': [2, 'always', ALLOWED_TYPES],
    'subject-case': [0],
    'scope-case': [2, 'always', ['lower-case', 'kebab-case']],
    'header-max-length': [2, 'always', 100],
  },
  ignores: [
    (message) => message.startsWith('Merge'),
    (message) => /chore: release/i.test(message),
    (message) => message.startsWith('chore(') && message.includes('release'),
  ],
};
