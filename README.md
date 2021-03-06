# 2020 소켓 프로그래밍 스터디

## 개발 환경

* **Windows 10**
* **Visual Studio 2019**
* **C++20**

## 코딩 컨벤션

* [convention.md](convention.md)

## TODO
* **해결 한 일감**
- [x] 로직단 / 라이브러리단 스레드 분리
- [x] 콘솔 로거 개발
- [x] 라이브러리 세션 풀 미리 할당되는 방식으로 변경
- [x] Broadcast 채팅 서버 개발(로그인 X)
- [x] 시나리오 테스트 작성(간단하게 테스트용, 구조화 X)
- [x] Redis 라이브러리 적용(동기)
- [x] Redis 라이브러리 적용(비동기)
- [x] 로그인 기능 구현
- [x] Redis 비동기 처리
- [x] receive 버퍼 넉넉하게 잡아서 복사 최소화
- [x] json 라이브러리 사용 및 config 적용
- [x] 테스트 클라이언트 적용(C#)
- [x] 설정 파일 로드
* **해야하는 일감**
- [ ] 파일 로그 구현
- [ ] 통신 모델을 언제든지 바꿀 수 있도록 수정
- [ ] 부모 - 자식구조로 코드 리팩토링
- [ ] 템플릿 적용 할 수 있다면 적용(굳이 안해도 되지만, 학습용)
- [ ] Read/Write Lock 적용
- [ ] 메모리 풀 적용
- [ ] 스레드 풀 적용
- [ ] 비동기 로그 구현
- [ ] 서버끼리 통신 구현
*  **라이브러리단 구현**
- [x] Select 방식
- [x] IOCP 방식
* **로직 서버 구현**
- [x] 채팅 서버
- [ ] 오목 서버
