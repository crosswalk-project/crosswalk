package org.xwalk.core.extension;

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
    /* Property "isWritable" is only meaningful for fields. */
    public boolean isWritable() default false;

    /* Property "isEventList" is only meaningful for fields,
     * methods will ignore this value.
     */
    public boolean isEventList() default false;

    /*
     * This property is only meaningful for functions/constructors.
     */
    public boolean isEntryPoint() default false;

    /*
     * This property is only meaningful for functions.
     */
    public boolean withPromise() default false;

    /*
     * This property is only meaningful for functions.
     * It should be either the custom function name
     * or just the function definition.
     */
    public String wrapArgs() default "";

    /*
     * This property is only meaningful for functions.
     * It should be either the custom function name
     * or just the function definition.
     */
    public String wrapReturns() default "";
}
