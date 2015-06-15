package org.xwalk.app.runtime.extension;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

@SuppressWarnings("javadoc")
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.PARAMETER})
public @interface JsCallback {
    public boolean isPromise() default false;
}
