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
const HEADER_REGEX = new RegExp(`^(?:${typePattern})(?:\\([^)]+\\))?!?: .+ \\(#\\d+\\)$`, 'i');

module.exports = {
  extends: ['@commitlint/config-conventional'],
  plugins: [
    {
      rules: {
        'header-contains-issue': ({ header }) => {
          const valid = HEADER_REGEX.test(header || '');
          return [
            valid,
            'commit header must follow "type: summary (#123)" and include an issue number'
          ];
        },
      },
    },
  ],
  rules: {
    'type-enum': [2, 'always', ALLOWED_TYPES],
    'subject-case': [0],
    'scope-case': [2, 'always', ['lower-case', 'kebab-case']],
    'header-max-length': [2, 'always', 100],
    'references-empty': [2, 'never'],
    'header-contains-issue': [2, 'always'],
  },
  ignores: [
    (message) => message.startsWith('Merge'),
    (message) => /chore: release/i.test(message),
    (message) => message.startsWith('chore(') && message.includes('release'),
  ],
};
