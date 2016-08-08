package uk.ac.cam.cheri;
import java.lang.annotation.*;

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.METHOD)
public @interface Sandbox {
        enum Scope {
                Global, Object, Method
        }
        Scope scope() default Scope.Global;
        String SandboxClass();
};

