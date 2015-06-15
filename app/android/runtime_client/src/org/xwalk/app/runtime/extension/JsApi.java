package org.xwalk.app.runtime.extension;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Annotated fields and method can be exposed to JavaScript.
 */
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.METHOD, ElementType.FIELD})
public @interface JsApi {
    /* Property "isWritable" is only meanful for fields. */
    public boolean isWritable() default false;

    /* Property "isEventList" is only meanful for fields,
     * methods will ignore this value.
     */
    public boolean isEventList() default false;
}
