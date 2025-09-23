module.exports = {
  extends: ['@commitlint/config-conventional'],
  rules: {
    'type-enum': [2, 'always', [
      'feat',
      'fix',
      'bugfix',
      'docs',
      'style',
      'refactor',
      'perf',
      'test',
      'build',
      'ci',
      'chore',
      'revert'
    ]],
    'subject-case': [0],
    'body-leading-blank': [1, 'always'],
    'footer-leading-blank': [1, 'always']
  }
};
