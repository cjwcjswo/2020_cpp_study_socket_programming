# 명명 규칙

## 파일 이름
헤더 파일과 클래스 파일의 이름은 Pascal Case로 통일한다.
```cpp
Main.cpp, Main.h, ...
```

## 상수
모든 컴파일 타임 상수는 대문자로 작성하며 _로 단어를 구분한다.
```cpp
const int MAX_USER_COUNT = 100;
```

## 전역 변수
전역 변수 앞에는 접두사 G를 붙인다.
```cpp
int GValue = 100;
```

## 멤버 변수
모든 멤버 변수 앞 접두사에는 m을 붙이고 이후 PascalCase로 통일한다.
```cpp
public:
int mAge = 0;
bool mIsMale = false;
```

## 함수 이름
함수 이름은 Pascal Case로 통일한다.
```cpp
void TestFunction()
{
}
```

## 구조체
구조체 명명 규칙 역시 클래스와 동일하게 따른다.

# 헤더파일 작성 규칙

## 클래스 선언 순서

```cpp
// 템플릿 선언시 규칙
// template 키워드와 <> 사이엔 공백 1칸
// class 키워드 대신 typename 키워드로 통일할 것
template <typename T>
class TypeName : public ParentTypeName
{
// 1. 필요시 static_assert

// 2. 매크로 집단
    UE4_MACROS
    OTHER_MACROS
    
// 3. friend 클래스가 있다면 선언
    friend class 선언
    
// 4. 해당 class에 종속적인 타입별칭이 필요하다면, 변수 선언에 앞서 미리 정의
private:
    using 타입 별칭 정의  
    enum, const
    
// 5. 멤버변수 선언
private:
    _memberVariableList; ...
    
// 6. 생성자/소멸자 선언
public:
    TypeName();
    ~TypeName();
    
// 7. 정적 멤버함수들 선언
public:
    static MemberFunctions(); ...
    
// 8. 가상 멤버함수들 선언
public:
    virtual MemberFunctions(); ...
    
// 9. 일반 멥버함수들 선언
public:
    Logical public Functions(); ...
    
// 10. getter/setter 멤버함수들 선언
public:
    Getter/Setter Functions(); ...
};

// 11. 글로벌 스페이스 변수, 함수들 선언
```

## 헤더 파일에서의 다른 헤더파일 포함은 최대한 지양해야 한다

인터페이스 구성상 시그너쳐에 필요한 타입 정보 전달은 최대한 전방선언(forward declaration)을 활용한다.

전방선언을 헤더의 글로벌 스페이스에 할 것인지, 해당 타입 앞에 붙이는 방식을 쓸 것인지는 상황에 맞게 자유롭게 선택한다.

```cpp
// 1) 다음과 같이 글로벌 스페이스에서 전방선언을 하는 것 OK
class UButton;
  
class TestProject: public UGsUIWindow
{
    GENERATED_BODY()
    
protected:
    UPROPERTY()
    class UButton* BtnOk;   // 2) 타입 앞에 전방선언 형식으로 사용해도 OK
```

# 기타 규칙

## NULL은 이제 그만, 'nullptr'로만 표현할 것

언리얼 엔진의 코드에서도 여전히 NULL이 관찰되긴 하지만, 계속해서 nullptr로 변경하는 중

## 멤버 변수는 '선언 즉시 초기화'하는 습관을 들이자
```cpp
UCLASS()
class TestProject : public UGsGameFlowBase
{
    GENERATED_BODY()
    
    UPROPERTY(Transient)
    UGsManagersGame*                mManagers            = nullptr;
  
    class GsStageManagerGame*      mStageManager        = nullptr;
    class GsContentsManagerGame*    mContentsGameManager = nullptr;
```
Lazy initialization이 필수적인 녀석들이 아니라면, 이건 그냥 습관을 들이자.

## Primitive type은 참조형 인자(함수, 템플릿 모두)로 사용하지 말자
(함수의 OUT 파라미터는 당연히 이 이야기에 포함되지 않음을 미리 밝혀둔다)

C++의 Primitive type으로는 다음의 것들이 있다.

* Integral types
* Floating-point types
* Boolean
* char / wchar_t
* void
 
흔히 보이는 코드 실수 중 하나가 "복사를 피하려면 (상수)참조로…"라는 일반론이다 (이는 이후의 SharedPtr에서도 동일하게 반복되는데)

일단 저 일반론은 크기 측면에서 Primitive type들에 대해선 아무런 효과도 없다.

Primitive type들은 모두 8바이트 이내의 크기를 가지며, Byte-padding에 의해 레지스터 입장에선 그냥 다 8바이트  (64비트 환경에서)

그런데 참조 역시 주소값을 취하기에 8바이트 크기를 가진다.

고로, 크기 측면에서는 아무런 의미가 없고…

되려 참조는 다음과 같은 단점을 가진다.

분명 call by value이면 충분한 것을 call by reference로 넘겨 변경될 여지가 남게된다.
참조로 넘어갔을 때 사용처에서 또 다른 간접 참조를 만들어 낼 수 있다.
분명 한군데만 넘겼는데, 마구마구 참조가 생성되어 수정될 수 있게 되는 것이다.
 

이는 언어론적 관점에서 캡슐화 붕괴를 의미하기도 한다.

## '정확한 상수성 부여'는 상상 이상으로 중요하다
> 참고 : https://docs.unrealengine.com/ko/Programming/Development/CodingStandard/#const%EC%A0%95%ED%99%95%EB%8F%84

위 페이지에 관련 예제코드들이 잘 정리되어 있기에, 예제 코드들은 링크로 대체

중요한 것은 "Context 에서 상태가 변하지 않는 것들에 대해 constness를 부여"하겠다는 의지와 이를 통한 습관을 만들어 나가는 것!

1) 코드의 안정성 증대 뿐 아니라

2) 컴파일러가 constness가 보장된 녀석들에 대해 최적화해주는 부분은 생각보다 크다.

## 인자를 가지는 생성자 함수는 '최대한 explicit' 키워드와 함께~

explicit은 명시적 생성자 호출을 위한 C++ language keyword이다.

이 말을 뒤집으면 explicit 키워드 없는 생성자는 암시적(implicit)으로 호출된다는 이야기이다.

## 'auto' 키워드는 특수한 몇몇 경우가 아니면 사용하지 말 것

1. 람다함수를 변수에 저장할 때 허용

```cpp
int a = 7, b = 3;

// 외부 변수 a와 b를 복사 캡쳐하고, int x를 파라미터로 받는 lambda 함수
// a * x + b 의 결과를 출력한다.
auto closureFunc = [a, b] (int x)
{
    cout << a * x + b << endl;
};
  
closureFunc(5);  // 7 * 5  + 3 = 38 를 출력
```


2. 전통적인 iterator 를 다룰 때 허용
```cpp
// iterator를 다룰 땐 OK
for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
{
    // ...
}
```

3. Ranged base For Loop에서는 부분 허용 ([key, value] 컨테이너만 허용)
```cpp
// Ranged-For Loop는 기본적으로 NOT OK, 사용하면 아니 댐
// TArray<class UGsUIQuestSlotMain*> MainSlotList;
for (auto slot : MainSlotList)
// for (UGsUIQuestSlotMain* slot : MainSlotList)로 변경 요망
{
    if (slot)
    {
        slot->UpdateQuestDist();
    }
} 

// 하지만, [key, value]를 다루는 경우엔 부분적 허용
// TMap<StatType, FGsStatInfo*> _statGroup;
for (auto& iter : _statGroup)
{
    delete iter.Value;
    iter.Value = nullptr;
}
```

위 3가지만 예외이지, 다른 모든 경우들엔 auto 불허!

auto는 직관성도 문제지만, auto 키워드와 주소 참조가 결합되면 코드의 정확도가 떨어지고, 문제를 일으킬 수도 있게 된다.
