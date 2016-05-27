package org.xwalk.core.internal;

public @interface XWalkAPI {
    Class<?> impl() default Object.class;
    Class<?> instance() default Object.class;
    boolean createInternally() default false;
    boolean createExternally() default false;
    boolean reservable() default false;
    boolean noInstance() default false;
    boolean isConst() default false;
    boolean delegate() default false;
    boolean disableReflectMethod() default false;
    Class<?> extendClass() default Object.class;
    String[] preWrapperLines() default {};
    String[] postWrapperLines() default {};
    String[] postBridgeLines() default {};
}
