# 실제 검증 결과 · 2026-10-13

환경: Linux-6.18.44-x86_64-with-glibc2.41

## gcc
g++ (Debian 14.2.0-19) 14.2.0

정상 실행: 9/9, 조건 검사 89회

- lifetime: 10회
- ownership: 13회
- polymorphism: 12회
- templates: 10회
- containers: 16회
- interfaces: 7회
- transaction: 8회
- allocation: 7회
- pimpl: 6회

## clang
clang version 17.0.0 (https://github.com/swiftlang/llvm-project.git 10999b6d034fe318f3d56c83bddb6572593a8bb0)

정상 실행: 9/9, 조건 검사 89회

- lifetime: 10회
- ownership: 13회
- polymorphism: 12회
- templates: 10회
- containers: 16회
- interfaces: 7회
- transaction: 8회
- allocation: 7회
- pimpl: 6회

## clang-sanitized
clang version 17.0.0 (https://github.com/swiftlang/llvm-project.git 10999b6d034fe318f3d56c83bddb6572593a8bb0)

정상 실행: 9/9, 조건 검사 89회

- lifetime: 10회
- ownership: 13회
- polymorphism: 12회
- templates: 10회
- containers: 16회
- interfaces: 7회
- transaction: 8회
- allocation: 7회
- pimpl: 6회

## 한계
Windows/MSVC·GPU·실제 게임 통합·경쟁 조건·성능은 미검증. 로그는 특정 실행 경로의 결과이며 언어 규칙의 증명이나 전체 코드 커버리지가 아닙니다.
